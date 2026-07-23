/* =========================================================
   ROWEN — VST store checkout
   ---------------------------------------------------------
   Turns the visitor's cart (assets/js/cart.js: a list of
   { key, qty } pairs, never a price) into a real Stripe
   Checkout Session and redirects the browser to Stripe's own
   hosted payment page. The browser can never influence what
   gets charged: every price comes from api/_lib/catalog.js on
   this server, looked up by key.

   WHAT THIS COVERS RIGHT NOW: a working "Checkout" button that
   actually charges the card and comes back to the site.

   WHAT README.md ALSO DESCRIBES BUT ISN'T BUILT YET: automatic
   license-key generation, a gated download link, a webhook-
   triggered confirmation email (api/stripe-webhook.js,
   api/_lib/email.js via Resend), an order-success.html page,
   and lost-order recovery by email (api/recover-order.js). None
   of those files exist yet — right now a successful payment just
   returns the customer to the site with a `?checkout=success`
   flag; it does not yet email or display a license/download.
   That's the next step once you're ready to set up Resend and
   decide where the installer file will be hosted (see README.md,
   "Selling a finished VST", for the full walkthrough).

   Requires the STRIPE_SECRET_KEY environment variable to be set
   in Vercel (Project -> Settings -> Environment Variables) --
   this file only ever reads it from process.env, never hardcodes
   or logs it.
   ========================================================= */

const Stripe = require('stripe');
const catalog = require('./_lib/catalog');

const MAX_LINE_ITEMS = 20;   // distinct products in one order
const MAX_QTY_PER_ITEM = 20; // sanity cap, not a real inventory limit

// Vercel normally pre-parses a JSON body into req.body. If this project's
// NODEJS_HELPERS env var is ever set to 0 (README flags this as required
// later, for the Stripe webhook's raw-body signature check, and it applies
// to every function in the project, not just that one) that pre-parsing
// stops happening everywhere. Reading the raw stream ourselves means this
// endpoint keeps working either way, today or after that's added.
function readJsonBody(req) {
  if (req.body && typeof req.body === 'object') return Promise.resolve(req.body);
  return new Promise((resolve, reject) => {
    let data = '';
    req.on('data', (chunk) => {
      data += chunk;
      if (data.length > 1e6) req.destroy(); // guard against an absurdly large body
    });
    req.on('end', () => {
      if (!data) return resolve({});
      try {
        resolve(JSON.parse(data));
      } catch (err) {
        reject(err);
      }
    });
    req.on('error', reject);
  });
}

function sendJson(res, status, body) {
  res.statusCode = status;
  res.setHeader('Content-Type', 'application/json');
  res.end(JSON.stringify(body));
}

module.exports = async function handler(req, res) {
  if (req.method !== 'POST') {
    return sendJson(res, 405, { error: 'Method not allowed.' });
  }

  if (!process.env.STRIPE_SECRET_KEY) {
    console.error('create-checkout-session: STRIPE_SECRET_KEY is not set in this environment.');
    return sendJson(res, 500, { error: 'Checkout is not configured yet.' });
  }

  let body;
  try {
    body = await readJsonBody(req);
  } catch (err) {
    return sendJson(res, 400, { error: 'Malformed request body.' });
  }

  const requested = Array.isArray(body.items) ? body.items : [];
  if (!requested.length) {
    return sendJson(res, 400, { error: 'Your cart is empty.' });
  }
  if (requested.length > MAX_LINE_ITEMS) {
    return sendJson(res, 400, { error: 'Too many different items in one order.' });
  }

  // Re-derive every line item from the server-only catalog -- the client
  // only ever sends a key + quantity, never a price (see cart.js), and any
  // key that isn't in the catalog (delisted product, tampered request) is
  // just skipped, the same way the cart drawer itself quietly skips items
  // no longer in SITE_CONFIG.vsts.
  const lineItems = [];
  for (const raw of requested) {
    const key = raw && typeof raw.key === 'string' ? raw.key : null;
    const product = key ? catalog[key] : null;
    if (!product) continue;

    const qty = raw && Number.isFinite(raw.qty) ? Math.floor(raw.qty) : NaN;
    if (!Number.isInteger(qty) || qty < 1 || qty > MAX_QTY_PER_ITEM) {
      return sendJson(res, 400, { error: `Invalid quantity for "${product.name}".` });
    }
    lineItems.push({ price: product.priceId, quantity: qty });
  }

  if (!lineItems.length) {
    return sendJson(res, 400, { error: 'None of the items in your cart are available anymore.' });
  }

  const origin = req.headers.origin || `https://${req.headers.host}`;
  const stripe = new Stripe(process.env.STRIPE_SECRET_KEY, { apiVersion: '2024-06-20' });

  try {
    const session = await stripe.checkout.sessions.create({
      mode: 'payment',
      line_items: lineItems,
      // No order-success.html yet (see header comment) -- land back on the
      // VST page with a flag main.js can use to show a thank-you toast.
      success_url: `${origin}/?checkout=success#vsts`,
      cancel_url: `${origin}/?checkout=cancelled#vsts`,
    });
    return sendJson(res, 200, { url: session.url });
  } catch (err) {
    console.error('create-checkout-session: Stripe error creating session', err);
    return sendJson(res, 500, { error: 'Could not start checkout. Please try again in a moment.' });
  }
};
