/* =========================================================
   LICENSE KEY GENERATION
   ---------------------------------------------------------
   Deterministic, not stored anywhere: a license key is built from an
   HMAC-SHA256 signature over (a short order tag derived from the Checkout
   Session ID) + product key, using a server-only secret (the LICENSE_SECRET
   environment variable). Given the same paid session + product, this always
   regenerates the exact same key -- so there's no database to keep in sync,
   lose, or leak. Payment validity is always re-checked against Stripe itself
   (is this session actually paid?), never against a locally stored list.

   IMPORTANT: this exact algorithm is duplicated in the plugin itself
   (src/licensing/LicenseVerifier.h in each product's repo) so the plugin can
   verify a key completely offline, with no server call. If you change
   anything about how a key is built here, the plugin-side copy must change
   identically or every previously-issued key stops working. See that file's
   header comment for the full format spec and the security trade-off of
   offline verification.

   Format: "<PREFIX>-XXXX-XXXX-XXXX-XXXX-XXXX"
     PREFIX = short per-product tag (see PRODUCT_PREFIXES below)
     the 20 X's = 8-char orderId (from the session ID) + 12-char signature
   ========================================================= */
const crypto = require('crypto');

// Must match the PLUGIN_CODE-style prefix each plugin's LicenseVerifier.h is
// built with. Add a new line here the same day a new product's key
// generation goes live -- never remove/change one, or every key ever issued
// for that product silently stops matching what its plugin expects.
const PRODUCT_PREFIXES = {
  'vocal-polish': 'RVP',
};

function getSecret(){
  const secret = process.env.LICENSE_SECRET;
  if (!secret) {
    throw new Error('LICENSE_SECRET environment variable is not set. Add it in your Vercel project settings -- see the README.');
  }
  return secret;
}

function computeOrderId(sessionId){
  return crypto.createHash('sha256').update(sessionId).digest('hex').toUpperCase().slice(0, 8);
}

function computeSignature(secret, orderId, productKey){
  const hmac = crypto.createHmac('sha256', secret);
  hmac.update(`${orderId}:${productKey}`);
  return hmac.digest('hex').toUpperCase().slice(0, 12);
}

function generateLicenseKey(sessionId, productKey){
  const secret = getSecret();
  const orderId = computeOrderId(sessionId);
  const signature = computeSignature(secret, orderId, productKey);
  const body = (orderId + signature).match(/.{1,4}/g).join('-'); // 20 chars -> 5 groups of 4
  const prefix = PRODUCT_PREFIXES[productKey] || 'ROWN';
  return `${prefix}-${body}`;
}

module.exports = { generateLicenseKey, PRODUCT_PREFIXES };
