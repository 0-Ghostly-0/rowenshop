/* =========================================================
   GET /api/beats
   ---------------------------------------------------------
   Returns the array of auto-approved beats as JSON — this is what
   main.js's renderBeats() fetches at runtime and merges with any
   beats listed by hand in config.js (CFG.beats). Returns an empty
   array (never an error) if nothing's been approved yet, or if Blob
   storage isn't configured — the Beats page should degrade to
   showing whatever's in CFG.beats (or the "coming soon" panel if
   that's empty too), never break the page.
   ========================================================= */
const { sendJson } = require('./_lib/http');
const { readJsonBlob, CATALOG_PATH } = require('./_lib/blobStore');

module.exports = async (req, res) => {
  res.setHeader('Cache-Control', 'public, max-age=0, s-maxage=30, stale-while-revalidate=300');
  try {
    const beats = await readJsonBlob(CATALOG_PATH);
    return sendJson(res, 200, beats || []);
  } catch (err) {
    // Missing BLOB_READ_WRITE_TOKEN or a transient Blob error shouldn't
    // take down the Beats page — just report no auto-approved beats.
    return sendJson(res, 200, []);
  }
};
