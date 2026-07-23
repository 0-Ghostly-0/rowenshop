/* =========================================================
   Manual replacements for Vercel's automatic req.body / req.query
   helpers.
   ---------------------------------------------------------
   The Stripe webhook handler (api/stripe-webhook.js) needs the
   exact raw, unaltered request bytes to verify Stripe's signature —
   any parsing (even just JSON.parse + re-stringify) can change
   whitespace/key order enough to break that check. Vercel's helpers
   only expose an already-parsed body, and there's no way to turn
   that off for just one function — it's a project-wide setting
   (NODEJS_HELPERS=0, see the README). So every function in this
   project does its own parsing with these two small helpers instead,
   rather than only the webhook being special-cased.
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
  const raw = await readRawBody(req);
  if (!raw.length) return {};
  try { return JSON.parse(raw.toString('utf8')); }
  catch (err) { return {}; }
}

function parseQuery(req){
  const parsed = new URL(req.url, 'http://placeholder.local');
  return Object.fromEntries(parsed.searchParams.entries());
}

// NODEJS_HELPERS=0 (see above) also turns off response.status()/.json() —
// they're part of the same helper bundle as request.body/query — so every
// function sends JSON through this instead of the Vercel convenience methods.
function sendJson(res, statusCode, obj){
  res.statusCode = statusCode;
  res.setHeader('Content-Type', 'application/json');
  res.end(JSON.stringify(obj));
}

module.exports = { readRawBody, parseJsonBody, parseQuery, sendJson };
