/* =========================================================
   POST /api/submit-beat
   ---------------------------------------------------------
   Receives a producer's beat submission (main.js submitBeatForm())
   AFTER the browser has already uploaded any files straight to Blob
   storage via /api/beat-upload — this endpoint only ever sees text
   fields plus the resulting file URLs, never raw file bytes, so it
   stays well under Vercel's request body size limit no matter how
   big the MP3 is.

   What it does:
     1. Validates the submission server-side (never trust the
        browser's own validation alone).
     2. Stores it as "pending/<id>.json" in Blob storage — nothing
        published yet.
     3. Emails BEAT_REVIEW_EMAIL (Rowen's inbox) via Resend with the
        full submission and two pre-signed links: Approve / Reject.
        Clicking Approve is the entire "add it to the site" step —
        see api/approve-beat.js.

   Needs these Vercel environment variables (see README):
     BLOB_READ_WRITE_TOKEN  — already set if VST downloads work
     RESEND_API_KEY         — already set if VST license emails work
     EMAIL_FROM             — optional, same as above
     BEAT_APPROVAL_SECRET   — NEW, any long random string
     BEAT_REVIEW_EMAIL      — NEW, the inbox that should get these
   ========================================================= */
const crypto = require('crypto');
const { parseJsonBody, sendJson } = require('./_lib/http');
const { writeJsonBlob, pendingJsonPath } = require('./_lib/blobStore');
const { signBeatAction } = require('./_lib/beatApproval');
const { sendBeatReviewEmail } = require('./_lib/email');

const LICENSE_OPTIONS = ['MP3 Lease', 'WAV Lease', 'Unlimited', 'Exclusive'];

function isBlank(v) {
  return v == null || String(v).trim() === '';
}

function validate(body) {
  if (isBlank(body.producerName)) return 'Producer / artist name is required.';
  if (isBlank(body.beatTitle)) return 'Beat title is required.';
  if (isBlank(body.mp3Url)) return 'An MP3 preview upload is required.';
  if (!Array.isArray(body.license) || !body.license.length || !body.license.every((l) => LICENSE_OPTIONS.includes(l))) {
    return 'Pick at least one valid license type.';
  }
  if (body.permission !== true) return 'Permission to feature this beat is required.';
  if (body.bpm != null && body.bpm !== '' && (isNaN(Number(body.bpm)) || Number(body.bpm) <= 0 || Number(body.bpm) > 400)) {
    return 'BPM looks invalid.';
  }
  if (body.price != null && body.price !== '' && (isNaN(Number(body.price)) || Number(body.price) < 0)) {
    return 'Price looks invalid.';
  }
  return null;
}

module.exports = async (req, res) => {
  if (req.method !== 'POST') {
    return sendJson(res, 405, { error: 'Method not allowed' });
  }

  let body;
  try {
    body = await parseJsonBody(req);
  } catch (err) {
    return sendJson(res, 400, { error: 'Could not read submission.' });
  }

  // Basic bot defense, mirroring the honeypot/timing check already run
  // client-side — never trust that check alone since it's trivial to
  // skip by calling this endpoint directly.
  if (body.gotcha) {
    return sendJson(res, 200, { ok: true }); // pretend success, drop silently
  }
  if (typeof body.elapsedMs === 'number' && body.elapsedMs < 1500) {
    return sendJson(res, 200, { ok: true });
  }

  const validationError = validate(body);
  if (validationError) {
    return sendJson(res, 400, { error: validationError });
  }

  const reviewEmail = process.env.BEAT_REVIEW_EMAIL;
  if (!reviewEmail) {
    return sendJson(res, 500, { error: 'Beat submissions are not fully set up yet (missing BEAT_REVIEW_EMAIL) — please try again later or reach out on Discord instead.' });
  }

  const submission = {
    producerName: String(body.producerName).trim(),
    bio: body.bio ? String(body.bio).trim() : '',
    socialLinks: body.socialLinks ? String(body.socialLinks).trim() : '',
    storeLink: body.storeLink ? String(body.storeLink).trim() : '',
    beatTitle: String(body.beatTitle).trim(),
    bpm: body.bpm ? Number(body.bpm) : null,
    key: body.key ? String(body.key).trim() : '',
    genre: body.genre ? String(body.genre).trim() : '',
    mood: body.mood ? String(body.mood).trim() : '',
    price: body.price ? Number(body.price) : null,
    license: body.license,
    wavLink: body.wavLink ? String(body.wavLink).trim() : '',
    photoUrl: body.photoUrl || null,
    coverUrl: body.coverUrl || null,
    mp3Url: body.mp3Url,
    submittedAt: new Date().toISOString()
  };

  const id = crypto.randomUUID();

  try {
    await writeJsonBlob(pendingJsonPath(id), submission);
  } catch (err) {
    return sendJson(res, 500, { error: 'Could not save submission — please try again or reach out on Discord.' });
  }

  const host = req.headers['x-forwarded-host'] || req.headers.host;
  // Vercel always sets x-forwarded-proto (to "https"), so this only
  // falls back to inspecting the raw socket when running outside
  // Vercel (e.g. a local dev server over plain HTTP during testing).
  const protocol = req.headers['x-forwarded-proto'] || (req.socket && req.socket.encrypted ? 'https' : 'http');
  const base = `${protocol}://${host}`;

  let approveUrl, rejectUrl;
  try {
    approveUrl = `${base}/api/approve-beat?id=${encodeURIComponent(id)}&sig=${signBeatAction(id, 'approve')}`;
    rejectUrl = `${base}/api/reject-beat?id=${encodeURIComponent(id)}&sig=${signBeatAction(id, 'reject')}`;
  } catch (err) {
    return sendJson(res, 500, { error: 'Beat submissions are not fully set up yet (missing BEAT_APPROVAL_SECRET) — please try again later or reach out on Discord instead.' });
  }

  try {
    await sendBeatReviewEmail({ to: reviewEmail, submission, approveUrl, rejectUrl });
  } catch (err) {
    // The submission is already safely stored even if the email fails
    // to send — not ideal (nobody gets notified), but better than
    // losing the submission outright. Still report success to the
    // producer since their part is done.
    return sendJson(res, 200, { ok: true, emailWarning: true });
  }

  return sendJson(res, 200, { ok: true });
};
