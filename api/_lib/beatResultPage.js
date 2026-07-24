/* =========================================================
   Tiny shared HTML page for api/approve-beat.js and
   api/reject-beat.js — these are plain email links a person clicks,
   not API calls a script reads, so they return a real (if minimal)
   HTML page rather than JSON.
   ========================================================= */
function sendResultPage(res, { title, message, ok }) {
  res.setHeader('Content-Type', 'text/html; charset=utf-8');
  res.statusCode = ok ? 200 : 400;
  res.end(`<!doctype html>
<html><head><meta charset="utf-8"><title>${title}</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  body{background:#0d0d0d;color:#f5f5f5;font-family:Arial,Helvetica,sans-serif;display:flex;min-height:100vh;align-items:center;justify-content:center;margin:0;padding:24px;box-sizing:border-box;}
  .card{max-width:420px;text-align:center;}
  .icon{font-size:40px;margin-bottom:12px;}
  h1{font-size:20px;margin:0 0 8px;}
  p{color:#9a9a9a;font-size:14px;line-height:1.6;margin:0;}
</style></head>
<body><div class="card">
  <div class="icon">${ok ? '✓' : '✕'}</div>
  <h1>${title}</h1>
  <p>${message}</p>
</div></body></html>`);
}

module.exports = { sendResultPage };
