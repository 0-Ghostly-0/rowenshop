/* =========================================================
   GET /api/order-status?session_id=cs_...
   ---------------------------------------------------------
   Called by order-success.html right after checkout. Verifies
   the Checkout Session with Stripe directly (never trusts the
   redirect alone — a URL can be visited without paying), and
   only then returns a license key + download link per product.
   ========================================================= */
const Stripe = require('stripe');
const catalog = require('./_lib/catalog');
const { generateLicenseKey } = require('./_lib/license');

module.exports = async (req, res) => {
  const secretKey = process.env.STRIPE_SECRET_KEY;
  if (!secretKey) {
    res.status(500).json({ error: 'Order lookup isn’t configured yet — missing STRIPE_SECRET_KEY.' });
    return;
  }
  const stripe = new Stripe(secretKey);

  const sessionId = req.query && req.query.session_id;
  if (!sessionId || typeof sessionId !== 'string') {
    res.status(400).json({ error: 'Missing session_id.' });
    return;
  }

  try {
    const session = await stripe.checkout.sessions.retrieve(sessionId, {
      expand: ['line_items.data.price']
    });
    if (session.payment_status !== 'paid') {
      res.status(402).json({ error: 'This order hasn’t completed payment yet.' });
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
    res.status(200).json({
      email: (session.customer_details && session.customer_details.email) || null,
      products
    });
  } catch (err) {
    console.error('order-status error:', err);
    res.status(404).json({ error: 'Order not found.' });
  }
};
