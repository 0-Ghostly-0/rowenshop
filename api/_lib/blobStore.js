/* =========================================================
   Small helpers around Vercel Blob storage, used by the automatic
   beat-submission pipeline (api/submit-beat.js, api/approve-beat.js,
   api/reject-beat.js, api/beats.js).
   ---------------------------------------------------------
   Needs the BLOB_READ_WRITE_TOKEN environment variable — Vercel sets
   this automatically once a Blob store is connected to the project
   (Vercel dashboard → Storage → your project already has one, since
   it's what serves the Rowen Fragment download link). @vercel/blob
   picks that env var up on its own; nothing to pass explicitly.

   Two kinds of thing live in Blob storage here:
     - "pending/<id>.json"          one per unapproved submission,
                                     deleted the moment it's approved
                                     or rejected.
     - "data/beats-catalog.json"    the single source of truth for
                                     every LIVE auto-published beat —
                                     read by api/beats.js, which the
                                     site's Beats page fetches at
                                     runtime (see main.js renderBeats()).
   Both are plain JSON blobs with a fixed, predictable pathname
   (addRandomSuffix: false) so they can be found again later just by
   name, instead of needing a database to remember a generated URL.
   ========================================================= */
const { put, list, del } = require('@vercel/blob');

// Fixed pathname for the single "live beats catalog" blob — shared by
// api/beats.js (reads it) and api/approve-beat.js (appends to it).
const CATALOG_PATH = 'catalog/beats.json';

// Pathname helpers for a pending submission's metadata JSON, shared by
// api/submit-beat.js (writes), api/approve-beat.js and
// api/reject-beat.js (read/delete).
function pendingJsonPath(id) {
  return `pending/${id}.json`;
}
function pendingPrefix(id) {
  return `pending/${id}`;
}

/* Finds a blob by its exact pathname (list() only supports prefix
   matching, so this narrows to an exact string match) and returns its
   metadata ({ url, pathname, ... }), or null if nothing's there yet. */
async function findBlob(pathname) {
  const { blobs } = await list({ prefix: pathname, limit: 1 });
  return blobs.find((b) => b.pathname === pathname) || null;
}

/* Finds a blob by exact pathname and returns its parsed JSON content,
   or null if it doesn't exist yet (e.g. no beat has ever been
   approved, or a submission id that's already been handled). */
async function readJsonBlob(pathname) {
  const blob = await findBlob(pathname);
  if (!blob) return null;
  const res = await fetch(blob.url);
  if (!res.ok) return null;
  return res.json();
}

/* Writes (or overwrites) a JSON blob at a fixed pathname.
   allowOverwrite is required by @vercel/blob whenever you intentionally
   reuse the same pathname (our catalog file gets overwritten on every
   approval) — without it, put() throws to protect against accidental
   collisions. */
async function writeJsonBlob(pathname, data) {
  await put(pathname, JSON.stringify(data, null, 2), {
    access: 'public',
    addRandomSuffix: false,
    allowOverwrite: true,
    contentType: 'application/json'
  });
}

/* Deletes the single blob at this exact pathname, if it exists. Used
   on approve, where we only want to remove the pending metadata JSON
   and leave the actual uploaded files alone — they're the live beat's
   assets now. */
async function deleteBlob(pathname) {
  const blob = await findBlob(pathname);
  if (blob) await del(blob.url);
}

/* Deletes every blob whose pathname starts with the given prefix —
   used on reject, to clean up a pending submission's metadata JSON
   AND its uploaded files together in one call (e.g. prefix
   "pending/<id>" matches both "pending/<id>.json" and
   "pending/<id>/mp3-....mp3", since both start with that string). */
async function deleteBlobsByPrefix(prefix) {
  const { blobs } = await list({ prefix });
  if (!blobs.length) return;
  await del(blobs.map((b) => b.url));
}

module.exports = {
  findBlob,
  readJsonBlob,
  writeJsonBlob,
  deleteBlob,
  deleteBlobsByPrefix,
  CATALOG_PATH,
  pendingJsonPath,
  pendingPrefix
};
