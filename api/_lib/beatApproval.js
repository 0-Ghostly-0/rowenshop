/* =========================================================
   Signs and verifies the one-click Approve / Reject links sent in
   the beat-submission review email (api/submit-beat.js builds them,
   api/approve-beat.js and api/reject-beat.js verify them).
   ---------------------------------------------------------
   Same idea as api/_lib/license.js: an HMAC-SHA256 of the submission
   id + which action ("approve" or "reject"), using a server-only
   secret (BEAT_APPROVAL_SECRET). Anyone with the resulting link can
   approve/reject that ONE submission and nothing else — they can't
   forge a signature for a different id or action without the secret,
   and there's nothing to look up or expire server-side (the pending
   submission itself disappearing after being handled is what makes
   the link a one-time thing in practice).
   ========================================================= */
const crypto = require('crypto');

function getSecret() {
  const secret = process.env.BEAT_APPROVAL_SECRET;
  if (!secret) {
    throw new Error('BEAT_APPROVAL_SECRET environment variable is not set. Add it in your Vercel project settings — see the README.');
  }
  return secret;
}

function signBeatAction(id, action) {
  const hmac = crypto.createHmac('sha256', getSecret());
  hmac.update(`${id}:${action}`);
  return hmac.digest('hex');
}

function verifyBeatAction(id, action, signature) {
  if (!id || !action || !signature) return false;
  let expected;
  try {
    expected = Buffer.from(signBeatAction(id, action), 'hex');
  } catch (err) {
    return false;
  }
  let given;
  try {
    given = Buffer.from(signature, 'hex');
  } catch (err) {
    return false;
  }
  if (expected.length !== given.length) return false;
  return crypto.timingSafeEqual(expected, given);
}

module.exports = { signBeatAction, verifyBeatAction };
