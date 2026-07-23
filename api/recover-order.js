/* =========================================================
   POST /api/recover-order  { email }
   ---------------------------------------------------------
   "Lost your download?" flow. Looks up completed VST purchases
   by the email the customer paid with — no account/password
   system needed. Uses only Stripe's standard Customer + Checkout
   Session list endpoints (both created automatically at checkout
   via customer_creation: 'always'), so there's nothing separate
   to keep in sync. Mainly a backup for the automatic email —
   see api/stripe-webhook.js.
   ========================================================= */
const Stripe = require('stripe');
const catalog = require('./_lib/catalog');
const { generateLicenseKey } = require('./_lib/license');
const { parseJsonBody, sendJson } = require('./_lib/http');

module.exports = async (req, res) => {
  if (req.method !== 'POST') {
    sendJson(res, 405, { error: 'Method not allowed.' });
    return;
  }
  const secretKey = process.env.STRIPE_SECRET_KEY;
  if (!secretKey) {
    sendJson(res, 500, { error: "Order lookup isn't configured yet — missing STRIPE_SECRET_KEY." });
    return;
  }
  const stripe = new Stripe(secretKey);

  const body = await parseJsonBody(req);
  const email = body && typeof body.email === 'string' ? body.email.trim() : '';
  if (!email || !/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email)) {
    sendJson(res, 400, { error: 'Enter a valid email address.' });
    return;
  }

  try {
    const customers = await stripe.customers.list({ email, limit: 10 });
    const orders = [];
    for (const customer of customers.data) {
      const sessions = await stripe.checkout.sessions.list({ customer: customer.id, limit: 20 });
      for (const summary of sessions.data) {
        if (summary.payment_status !== 'paid') continue;
        const session = await stripe.checkout.sessions.retrieve(summary.id, {
          expand: ['line_items.data.price']
        });
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
        if (products.length) orders.push({ sessionId: session.id, products });
      }
    }
    sendJson(res, 200, { orders });
  } catch (err) {
    console.error('recover-order error:', err);
    sendJson(res, 500, { error: 'Could not look up your orders — please try again in a moment.' });
  }
};
