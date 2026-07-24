/* =========================================================
   LICENSE KEY GENERATION
   ---------------------------------------------------------
   Deterministic, not stored anywhere: a license key is an
   HMAC-SHA256 of (Checkout Session ID + product key) using a
   server-only secret (the LICENSE_SECRET environment variable).
   Given the same paid session + product, this always regenerates
   the exact same key — so there's no database to keep in sync,
   lose, or leak. Validity is always re-checked against Stripe
   itself (is this session actually paid?), never against a
   locally stored list.
   ========================================================= */
const crypto = require('crypto');

function getSecret(){
  const secret = process.env.LICENSE_SECRET;
  if (!secret) {
    throw new Error('LICENSE_SECRET environment variable is not set. Add it in your Vercel project settings — see the README.');
  }
  return secret;
}

function generateLicenseKey(sessionId, productKey){
  const hmac = crypto.createHmac('sha256', getSecret());
  hmac.update(`${sessionId}:${productKey}`);
  const hex = hmac.digest('hex').toUpperCase().slice(0, 20);
  return hex.match(/.{1,4}/g).join('-');
}

module.exports = { generateLicenseKey };
