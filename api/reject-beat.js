/* =========================================================
   GET /api/reject-beat?id=...&sig=...
   ---------------------------------------------------------
   The "Reject" link from the beat-submission review email. Discards
   the pending submission entirely — its metadata JSON AND its
   uploaded files (mp3/cover/photo), since none of them will ever be
   used. Nothing is published, nothing is kept, and the producer isn't
   notified either way (same as a review that never gets added today).
   ========================================================= */
const { parseQuery } = require('./_lib/http');
const { readJsonBlob, deleteBlobsByPrefix, pendingJsonPath, pendingPrefix } = require('./_lib/blobStore');
const { verifyBeatAction } = require('./_lib/beatApproval');
const { sendResultPage } = require('./_lib/beatResultPage');

module.exports = async (req, res) => {
  const { id, sig } = parseQuery(req);

  let validSig;
  try {
    validSig = verifyBeatAction(id, 'reject', sig);
  } catch (err) {
    return sendResultPage(res, { ok: false, title: 'Not set up yet', message: 'BEAT_APPROVAL_SECRET is missing from this project\'s environment variables — add it in Vercel, then this link will work.' });
  }
  if (!validSig) {
    return sendResultPage(res, { ok: false, title: 'Invalid link', message: 'This reject link is invalid or has been tampered with.' });
  }

  let submission;
  try {
    submission = await readJsonBlob(pendingJsonPath(id));
  } catch (err) {
    return sendResultPage(res, { ok: false, title: 'Something went wrong', message: 'Could not reach storage to look up this submission. Try again in a moment.' });
  }
  if (!submission) {
    return sendResultPage(res, { ok: true, title: 'Already handled', message: 'This submission has already been approved or rejected — there\'s nothing left to do.' });
  }

  try {
    await deleteBlobsByPrefix(pendingPrefix(id));
  } catch (err) {
    return sendResultPage(res, { ok: false, title: 'Something went wrong', message: 'Could not fully discard this submission — try clicking the link again in a moment.' });
  }

  return sendResultPage(res, {
    ok: true,
    title: 'Rejected',
    message: `"${submission.beatTitle}" was discarded — nothing was published.`
  });
};
