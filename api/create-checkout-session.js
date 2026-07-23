/* =========================================================
   POST /api/create-checkout-session
   ---------------------------------------------------------
   Takes the visitor's cart ({ items: [{ key, qty }] }) and
   creates a real multi-item Stripe Checkout Session, returning
   its URL for the browser to redirect to.

   Security note: the browser only ever sends a product KEY and
   quantity — never a price. The actual Stripe Price ID always
   comes from the server-side catalog (api/_lib/catalog.js), so
   nothing the client sends can change what gets charged.
   ========================================================= */
const Stripe = require('stripe');
const catalog = require('./_lib/catalog');

module.exports = async (req, res) => {
  if (req.method !== 'POST') {
    res.status(405).json({ error: 'Method not allowed.' });
    return;
  }

  const secretKey = process.env.STRIPE_SECRET_KEY;
  if (!secretKey) {
    res.status(500).json({ error: 'Checkout isn’t configured yet — missing STRIPE_SECRET_KEY.' });
    return;
  }
  const stripe = new Stripe(secretKey);

  let body = req.body;
  if (typeof body === 'string') {
    try { body = JSON.parse(body); } catch (err) { body = {}; }
  }
  const items = Array.isArray(body && body.items) ? body.items : [];
  if (!items.length) {
    res.status(400).json({ error: 'Your cart is empty.' });
    return;
  }
  if (items.length > 20) {
    res.status(400).json({ error: 'Too many distinct items in one order.' });
    return;
  }

  const line_items = [];
  for (const item of items) {
    const key = item && typeof item.key === 'string' ? item.key : null;
    const qty = Math.max(1, Math.min(99, Math.floor(Number(item && item.qty)) || 1));
    const product = key ? catalog[key] : null;
    if (!product || !product.priceId) {
      res.status(400).json({ error: `"${key || 'One of the items'}" isn’t available for purchase right now.` });
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
    res.status(200).json({ url: session.url });
  } catch (err) {
    console.error('create-checkout-session error:', err);
    res.status(500).json({ error: 'Could not start checkout — please try again in a moment.' });
  }
};
