/* =========================================================
   POST /api/stripe-webhook
   ---------------------------------------------------------
   Stripe calls this automatically right after a Checkout Session
   completes. This is the "real" delivery mechanism for license
   keys/downloads — it doesn't depend on the customer's browser
   successfully loading order-success.html (closed tab, flaky
   connection, etc. all still work, since Stripe fires this
   server-to-server).

   Setup (see README): in the Stripe Dashboard, add a webhook
   endpoint pointing at https://rowen.work/api/stripe-webhook,
   subscribed to "checkout.session.completed", then copy its
   signing secret into STRIPE_WEBHOOK_SECRET.

   IMPORTANT: this requires NODEJS_HELPERS=0 (see api/_lib/http.js)
   so we can read the exact raw request body — Stripe's signature
   check fails if the body has been parsed and re-serialized even
   slightly differently (whitespace, key order, etc.).

   If sendLicenseEmail() throws (e.g. a transient Resend hiccup),
   we return 500 on purpose — Stripe automatically retries webhook
   delivery on non-2xx responses, so a flaky send now can still
   succeed on a later retry without any extra code here.
   ========================================================= */
const Stripe = require('stripe');
const catalog = require('./_lib/catalog');
const { generateLicenseKey } = require('./_lib/license');
const { readRawBody, sendJson } = require('./_lib/http');
const { sendLicenseEmail } = require('./_lib/email');

module.exports = async (req, res) => {
  if (req.method !== 'POST') {
    sendJson(res, 405, { error: 'Method not allowed.' });
    return;
  }

  const secretKey = process.env.STRIPE_SECRET_KEY;
  const webhookSecret = process.env.STRIPE_WEBHOOK_SECRET;
  if (!secretKey || !webhookSecret) {
    console.error('stripe-webhook: missing STRIPE_SECRET_KEY or STRIPE_WEBHOOK_SECRET.');
    sendJson(res, 500, { error: 'Webhook isn’t configured yet.' });
    return;
  }
  const stripe = new Stripe(secretKey);

  const rawBody = await readRawBody(req);
  const signature = req.headers['stripe-signature'];

  let event;
  try {
    event = stripe.webhooks.constructEvent(rawBody, signature, webhookSecret);
  } catch (err) {
    console.error('stripe-webhook: signature verification failed:', err.message);
    sendJson(res, 400, { error: `Webhook signature verification failed: ${err.message}` });
    return;
  }

  if (event.type !== 'checkout.session.completed') {
    // Not a fatal condition — just nothing for us to do with this event type.
    sendJson(res, 200, { received: true, ignored: event.type });
    return;
  }

  try {
    const summary = event.data.object;
    const session = await stripe.checkout.sessions.retrieve(summary.id, {
      expand: ['line_items.data.price']
    });

    if (session.payment_status !== 'paid') {
      sendJson(res, 200, { received: true, skipped: 'not paid' });
      return;
    }

    const email = (session.customer_details && session.customer_details.email) || null;
    const products = [];
    for (const li of (session.line_items && session.line_items.data) || []) {
      const priceId = li.price && li.price.id;
      const key = Object.keys(catalog).find((k) => catalog[k].priceId === priceId);
      if (!key) continue;
      const product = catalog[key];
      products.push({
        key,
        name: product.name,
        licenseKey: generateLicenseKey(session.id, key),
        downloadUrl: product.downloadUrl
      });
    }

    if (!email || !products.length) {
      // Nothing we can email — order-success.html / recover-order.js remain available.
      sendJson(res, 200, { received: true, skipped: 'no email or no matching products' });
      return;
    }

    await sendLicenseEmail({ to: email, products });
    sendJson(res, 200, { received: true, emailed: true });
  } catch (err) {
    console.error('stripe-webhook: error handling checkout.session.completed:', err);
    // Non-2xx on purpose so Stripe retries this webhook later.
    sendJson(res, 500, { error: 'Could not process this event — will retry.' });
  }
};
