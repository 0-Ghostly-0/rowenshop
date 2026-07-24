const Stripe = require('stripe');
const catalog = require('./_lib/catalog');
const { parseJsonBody, sendJson } = require('./_lib/http');

module.exports = async (req, res) => {
  if (req.method !== 'POST') {
    sendJson(res, 405, { error: 'Method not allowed.' });
    return;
  }

  const secretKey = process.env.STRIPE_SECRET_KEY;
  if (!secretKey) {
    sendJson(res, 500, { error: "Checkout isn't configured yet — missing STRIPE_SECRET_KEY." });
    return;
  }
  const stripe = new Stripe(secretKey);

  const body = await parseJsonBody(req);
  const items = Array.isArray(body && body.items) ? body.items : [];
  if (!items.length) { sendJson(res, 400, { error: 'Your cart is empty.' }); return; }
  if (items.length > 20) { sendJson(res, 400, { error: 'Too many distinct items in one order.' }); return; }

  const line_items = [];
  for (const item of items) {
    const key = item && typeof item.key === 'string' ? item.key : null;
    const qty = Math.max(1, Math.min(99, Math.floor(Number(item && item.qty)) || 1));
    const product = key ? catalog[key] : null;
    if (!product || !product.priceId) {
      sendJson(res, 400, { error: `"${key || 'One of the items'}" isn't available for purchase right now.` });
      return;
    }
    line_items.push({ price: product.priceId, quantity: qty });
  }

  const origin = req.headers.origin || `https://${req.headers.host}`;

  try {
    const session = await stripe.checkout.sessions.create({
      mode: 'payment',
      line_items,
      success_url: `${origin}/order-success.html?session_id={CHECKOUT_SESSION_ID}`,
      cancel_url: `${origin}/#vsts`,
      customer_creation: 'always',
      billing_address_collection: 'auto'
    });
    sendJson(res, 200, { url: session.url });
  } catch (err) {
    console.error('create-checkout-session error:', err);
    sendJson(res, 500, { error: 'Could not start checkout — please try again in a moment.' });
  }
};
