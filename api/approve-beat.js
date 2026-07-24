/* =========================================================
   GET /api/approve-beat?id=...&sig=...
   ---------------------------------------------------------
   This is the "Approve & publish" link from the beat-submission
   review email (api/_lib/email.js buildBeatReviewEmailHtml). Clicking
   it is the entire publish step:
     1. Verify the signature actually matches this id+"approve" (see
        api/_lib/beatApproval.js) — without the right
        BEAT_APPROVAL_SECRET, nobody can forge a working link.
     2. Read the pending submission back out of Blob storage.
     3. Turn it into a beat-catalog entry (matching the shape
        buildBeatCard() in main.js already knows how to render) and
        append it to the live catalog blob.
     4. Delete the pending submission's metadata JSON — but NOT its
        uploaded files, which are now the live beat's actual assets.
   The site picks this up the moment someone next loads the Beats
   page (main.js fetches /api/beats at runtime) — no redeploy, no
   waiting on a build.
   ========================================================= */
const { parseQuery } = require('./_lib/http');
const { readJsonBlob, writeJsonBlob, deleteBlob, CATALOG_PATH, pendingJsonPath } = require('./_lib/blobStore');
const { verifyBeatAction } = require('./_lib/beatApproval');
const { sendResultPage } = require('./_lib/beatResultPage');

// Pulls the first link-looking thing out of a newline-separated
// "social links" textarea, so an auto-published beat still has SOME
// clickable producer credit even if they didn't fill in the separate
// storeLink field.
function firstLink(text) {
  if (!text) return null;
  const match = String(text).match(/https?:\/\/\S+/);
  return match ? match[0] : null;
}

module.exports = async (req, res) => {
  const { id, sig } = parseQuery(req);

  let validSig;
  try {
    validSig = verifyBeatAction(id, 'approve', sig);
  } catch (err) {
    return sendResultPage(res, { ok: false, title: 'Not set up yet', message: 'BEAT_APPROVAL_SECRET is missing from this project\'s environment variables — add it in Vercel, then this link will work.' });
  }
  if (!validSig) {
    return sendResultPage(res, { ok: false, title: 'Invalid link', message: 'This approve link is invalid or has been tampered with.' });
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

  const entry = {
    title: submission.beatTitle,
    audio: submission.mp3Url,
    bpm: submission.bpm || undefined,
    key: submission.key || undefined,
    tags: [submission.genre, submission.mood].filter(Boolean),
    price: submission.price != null ? submission.price : 12,
    cover: submission.coverUrl || undefined,
    wavLink: submission.wavLink || undefined,
    license: submission.license,
    producer: {
      name: submission.producerName,
      link: submission.storeLink || firstLink(submission.socialLinks) || undefined,
      photo: submission.photoUrl || undefined
    },
    approvedAt: new Date().toISOString()
  };

  try {
    const catalog = (await readJsonBlob(CATALOG_PATH)) || [];
    catalog.push(entry);
    await writeJsonBlob(CATALOG_PATH, catalog);
    await deleteBlob(pendingJsonPath(id));
  } catch (err) {
    return sendResultPage(res, { ok: false, title: 'Something went wrong', message: 'Could not publish this beat — try clicking the link again in a moment.' });
  }

  return sendResultPage(res, {
    ok: true,
    title: 'Approved & published',
    message: `"${entry.title}" will show up on rowen.work within a few seconds — no further steps needed.`
  });
};
