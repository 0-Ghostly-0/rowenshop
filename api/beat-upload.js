/* =========================================================
   POST /api/beat-upload
   ---------------------------------------------------------
   Authorizes direct-from-browser uploads to Vercel Blob storage for
   the producer beat-submission form (main.js submitBeatForm()).

   Why files go straight to Blob instead of through a server function:
   Vercel serverless functions cap request bodies at a few MB, well
   under the 15MB the form allows for an MP3. @vercel/blob's client
   upload() helper (loaded in the browser from a CDN — see main.js)
   handles this by asking THIS route for a short-lived upload token,
   then PUTting the file bytes directly to Blob storage itself. This
   route never sees the file's actual bytes, only its name/type/size
   up front (to approve or reject the upload) — handleUpload() from
   '@vercel/blob/client' does the token issuing/verification.

   Needs BLOB_READ_WRITE_TOKEN (see api/_lib/blobStore.js — same env
   var, already required for the VST download links).
   ========================================================= */
const { handleUpload } = require('@vercel/blob/client');
const { parseJsonBody, sendJson } = require('./_lib/http');

const ALLOWED_TYPES = [
  'audio/mpeg',
  'audio/mp3',
  'image/png',
  'image/jpeg',
  'image/webp',
  'image/gif'
];
const MAX_BYTES = 15 * 1024 * 1024;

module.exports = async (req, res) => {
  if (req.method !== 'POST') {
    return sendJson(res, 405, { error: 'Method not allowed' });
  }

  try {
    const body = await parseJsonBody(req);
    const jsonResponse = await handleUpload({
      body,
      request: req,
      onBeforeGenerateToken: async (pathname) => {
        // Only ever used for beat-submission uploads — reject anything
        // that doesn't look like it's going into our own pending/
        // folder, so this token-issuing route can't be repurposed to
        // upload arbitrary files elsewhere in the store.
        if (!pathname.startsWith('pending/')) {
          throw new Error('Unexpected upload path.');
        }
        return {
          allowedContentTypes: ALLOWED_TYPES,
          maximumSizeInBytes: MAX_BYTES,
          addRandomSuffix: true
        };
      },
      onUploadCompleted: async () => {
        // No action needed — the browser already gets the final blob
        // URL back directly from upload() and sends it to
        // /api/submit-beat itself once all three (optional) files are
        // done. This callback only fires on a real deployment (Blob
        // calls back over the internet, so it's a no-op on localhost),
        // and nothing here depends on it running.
      }
    });
    return sendJson(res, 200, jsonResponse);
  } catch (err) {
    return sendJson(res, 400, { error: err.message || 'Upload authorization failed.' });
  }
};
