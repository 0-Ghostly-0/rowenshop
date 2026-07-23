/* =========================================================
   SERVER-SIDE VST CATALOG — private, never sent to the browser
   ---------------------------------------------------------
   assets/js/config.js drives what visitors SEE (images,
   description, price, "Add to Cart"). This file drives what
   they get AFTER paying: it maps each product's "key" (must
   match the key on that same entry in config.js) to its real
   Stripe Price ID and the private download URL.

   Checkout, order-status, and order-recovery all look up prices
   and download links from HERE, not from anything the browser
   sends — so a visitor can never trick checkout into charging a
   different price, and a download link is never revealed until
   Stripe confirms that product was actually paid for.

   See the README section "Selling a finished VST" for the full
   walkthrough of what to fill in here when a plugin is ready.
   ========================================================= */
module.exports = {
  // 'vocal-polish': {
  //   name: 'Rowen Vocal Polish',
  //   priceId: 'price_xxxxxxxxxxxxx',      // from the Stripe Dashboard, NOT a Payment Link
  //   downloadUrl: 'https://example.com/path/to/rowen-vocal-polish-v1.0.0.zip'
  // }
};
