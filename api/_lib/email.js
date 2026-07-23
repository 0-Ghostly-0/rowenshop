/* =========================================================
   sendLicenseEmail({ to, products })
   ---------------------------------------------------------
   Sends the "here's your download" email right after a paid
   Checkout Session, via Resend's plain HTTP API (no SDK needed —
   Vercel's Node runtime has a built-in global fetch).

   Env vars used:
     RESEND_API_KEY  — from resend.com/api-keys
     EMAIL_FROM      — a verified sender, e.g. "Rowen <hello@rowen.work>".
                        Falls back to Resend's shared test address if unset,
                        which only works for sending to your OWN Resend
                        account email — fine for a first test, not for
                        real customers. See README for verifying a domain.

   Called from api/stripe-webhook.js only, after Stripe confirms
   payment_status === 'paid'. Throws on failure so the caller can
   return a non-2xx status and let Stripe retry the webhook later.
   ========================================================= */

function escapeHtml(str) {
  return String(str == null ? '' : str).replace(/[&<>"']/g, (ch) => ({
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    "'": '&#39;'
  }[ch]));
}

function buildEmailHtml(products) {
  const rows = products.map((p) => `
    <tr>
      <td style="padding:20px 0;border-top:1px solid #2a2a2a;">
        <div style="font-family:Arial,Helvetica,sans-serif;color:#f5f5f5;font-size:16px;font-weight:600;margin-bottom:8px;">
          ${escapeHtml(p.name)}
        </div>
        <div style="font-family:'Courier New',Courier,monospace;color:#c9a6ff;font-size:14px;letter-spacing:0.5px;background:#1a1a1a;border:1px solid #333;border-radius:6px;padding:10px 14px;display:inline-block;margin-bottom:12px;">
          ${escapeHtml(p.licenseKey)}
        </div>
        <div>
          <a href="${escapeHtml(p.downloadUrl)}" style="display:inline-block;background:#c9a6ff;color:#111;font-family:Arial,Helvetica,sans-serif;font-size:14px;font-weight:700;text-decoration:none;padding:10px 20px;border-radius:8px;">
            Download ${escapeHtml(p.name)}
          </a>
        </div>
      </td>
    </tr>
  `).join('');

  return `
  <div style="background:#0d0d0d;padding:32px 16px;">
    <table role="presentation" width="100%" style="max-width:520px;margin:0 auto;border-collapse:collapse;">
      <tr>
        <td style="padding-bottom:24px;">
          <div style="font-family:Arial,Helvetica,sans-serif;color:#fff;font-size:20px;font-weight:700;">Rowen</div>
          <div style="font-family:Arial,Helvetica,sans-serif;color:#9a9a9a;font-size:14px;margin-top:4px;">
            Thanks for your order — here's your license and download.
          </div>
        </td>
      </tr>
      ${rows}
      <tr>
        <td style="padding-top:24px;">
          <div style="font-family:Arial,Helvetica,sans-serif;color:#7a7a7a;font-size:12px;line-height:1.6;">
            Keep this email for your records — your license key regenerates the same way from your order,
            but this is the fastest way to find it again. Lost this email? Use the "Lost your download?"
            form on the order confirmation page with the email address you paid with.
          </div>
        </td>
      </tr>
    </table>
  </div>`;
}

async function sendLicenseEmail({ to, products }) {
  if (!to) throw new Error('sendLicenseEmail: missing "to" address.');
  if (!Array.isArray(products) || !products.length) {
    throw new Error('sendLicenseEmail: no products to include.');
  }

  const apiKey = process.env.RESEND_API_KEY;
  if (!apiKey) throw new Error('sendLicenseEmail: missing RESEND_API_KEY.');

  const from = process.env.EMAIL_FROM || 'Rowen <onboarding@resend.dev>';
  const subject = products.length === 1
    ? `Your download: ${products[0].name}`
    : `Your ${products.length} downloads from Rowen`;

  const res = await fetch('https://api.resend.com/emails', {
    method: 'POST',
    headers: {
      Authorization: `Bearer ${apiKey}`,
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      from,
      to,
      subject,
      html: buildEmailHtml(products)
    })
  });

  if (!res.ok) {
    const text = await res.text().catch(() => '');
    throw new Error(`sendLicenseEmail: Resend responded ${res.status} — ${text}`);
  }
}

module.exports = { sendLicenseEmail, buildEmailHtml, escapeHtml };
