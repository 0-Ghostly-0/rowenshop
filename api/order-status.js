/* =========================================================
   GET /api/order-status?session_id=cs_...
   ---------------------------------------------------------
   Called by order-success.html right after checkout. Verifies
   the Checkout Session with Stripe directly (never trusts the
   redirect alone — a URL can be visited without paying), and
   only then returns a license key + download link per product.

   Note: this is a convenience for showing the key on-screen right
   away. The email sent from api/stripe-webhook.js is the delivery
   that doesn't depend on the customer ever loading this page.
   ========================================================= */
const Stripe = require('stripe');
const catalog = require('./_lib/catalog');
const { generateLicenseKey } = require('./_lib/license');
const { parseQuery, sendJson } = require('./_lib/http');

module.exports = async (req, res) => {
  const secretKey = process.env.STRIPE_SECRET_KEY;
  if (!secretKey) {
    sendJson(res, 500, { error: "Order lookup isn't configured yet — missing STRIPE_SECRET_KEY." });
    return;
  }
  const stripe = new Stripe(secretKey);

  const query = parseQuery(req);
  const sessionId = query.session_id;
  if (!sessionId) {
    sendJson(res, 400, { error: 'Missing session_id.' });
    return;
  }

  try {
    const session = await stripe.checkout.sessions.retrieve(sessionId, {
      expand: ['line_items.data.price']
    });
    if (session.payment_status !== 'paid') {
      sendJson(res, 402, { error: "This order hasn't completed payment yet." });
      return;
    }
    const products = [];
    for (const li of (session.line_items && session.line_items.data) || []) {
      const priceId = li.price && li.price.id;
      const key = Object.keys(catalog).find(k => catalog[k].priceId === priceId);
      if (!key) continue;
      const product = catalog[key];
      products.push({
        key,
        name: product.name,
        licenseKey: generateLicenseKey(session.id, key),
        downloadUrl: product.downloadUrl
      });
    }
    sendJson(res, 200, {
      email: (session.customer_details && session.customer_details.email) || null,
      products
    });
  } catch (err) {
    console.error('order-status error:', err);
    sendJson(res, 404, { error: 'Order not found.' });
  }
};
