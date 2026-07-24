/* =========================================================
   Request/response helpers that work whether or not NODEJS_HELPERS=0
   is set on this Vercel project.
   ---------------------------------------------------------
   Only api/stripe-webhook.js truly needs NODEJS_HELPERS=0 — it needs
   the exact raw, unaltered request bytes to verify Stripe's signature,
   and Vercel's automatic helpers only expose an already-parsed body.
   That setting is project-wide, not per-function, so it's tempting to
   make every function do its own manual parsing to match. That was
   this file's original approach, but it has a serious failure mode:
   if NODEJS_HELPERS is NOT actually set to 0 yet, Vercel's default
   helper has ALREADY read the request body to populate req.body
   before a function's handler even runs — which means the stream is
   already fully consumed. A function that then tries to read that
   same stream again (req.on('data', ...)) never receives another
   'data' or 'end' event, so it just hangs until Vercel kills it —
   producing a generic FUNCTION_INVOCATION_FAILED with no useful error
   message. That's exactly what broke checkout here: the raw-body
   helpers below were added for the webhook, but NODEJS_HELPERS=0
   hadn't been set on the project yet, so every OTHER function
   (checkout, order-status, recover-order) started hanging too.

   The fix: parseJsonBody/parseQuery/sendJson below first check
   whether Vercel's own req.body/req.query/res.status already exist
   and use them directly if so — only falling back to manual
   stream-reading when they're genuinely absent (i.e. NODEJS_HELPERS=0
   really is set). This makes checkout/order-status/recover-order work
   correctly EITHER WAY, so setting up the webhook can never again
   break the rest of the store. Only stripe-webhook.js still calls
   readRawBody() directly (it must — there's no substitute for the raw
   bytes), which is why that one function specifically still requires
   NODEJS_HELPERS=0 to work.
   ========================================================= */
const { URL } = require('url');

function readRawBody(req){
  return new Promise((resolve, reject) => {
    const chunks = [];
    req.on('data', (chunk) => chunks.push(chunk));
    req.on('end', () => resolve(Buffer.concat(chunks)));
    req.on('error', reject);
  });
}

async function parseJsonBody(req){
  // NODEJS_HELPERS not set to 0 → Vercel already parsed the body for us
  // (and already consumed the stream doing it) — use what it gives us
  // instead of trying to read the stream again.
  if (req.body !== undefined) {
    if (typeof req.body === 'string') {
      try { return JSON.parse(req.body); } catch (err) { return {}; }
    }
    return req.body || {};
  }
  // NODEJS_HELPERS=0 → nothing has touched the stream yet, safe to read it.
  const raw = await readRawBody(req);
  if (!raw.length) return {};
  try { return JSON.parse(raw.toString('utf8')); }
  catch (err) { return {}; }
}

function parseQuery(req){
  if (req.query) return req.query;
  const parsed = new URL(req.url, 'http://placeholder.local');
  return Object.fromEntries(parsed.searchParams.entries());
}

function sendJson(res, statusCode, obj){
  if (typeof res.status === 'function' && typeof res.json === 'function') {
    res.status(statusCode).json(obj);
    return;
  }
  res.statusCode = statusCode;
  res.setHeader('Content-Type', 'application/json');
  res.end(JSON.stringify(obj));
}

module.exports = { readRawBody, parseJsonBody, parseQuery, sendJson };
