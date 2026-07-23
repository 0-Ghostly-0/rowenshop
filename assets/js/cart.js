/* =========================================================
   ROWEN — VST cart
   ---------------------------------------------------------
   A small standalone module (loaded before main.js) that owns
   the shopping-cart state for the VST store: what's in it,
   how many, and persisting that across page loads/tabs via
   localStorage. It does NOT touch the DOM — main.js reads
   window.RowenCart to render the cart badge/drawer and to
   build the checkout request.

   Cart entries only ever store { key, qty } — the actual
   product name/price/image always comes from SITE_CONFIG.vsts
   at render time, so the cart can never go stale or show a
   price that doesn't match what checkout will actually charge.
   ========================================================= */
(function(){
  "use strict";

  const STORAGE_KEY = 'rowen_vst_cart';
  let items = load();

  function load(){
    try{
      const raw = window.localStorage.getItem(STORAGE_KEY);
      const parsed = raw ? JSON.parse(raw) : [];
      if (!Array.isArray(parsed)) return [];
      return parsed.filter(it => it && typeof it.key === 'string' && Number.isFinite(it.qty) && it.qty > 0);
    }catch(err){
      return [];
    }
  }

  function persist(){
    try{ window.localStorage.setItem(STORAGE_KEY, JSON.stringify(items)); }catch(err){ /* storage unavailable — cart just won't persist */ }
    notify();
  }

  const listeners = new Set();
  function notify(){ listeners.forEach(fn => { try{ fn(getItems()); }catch(err){} }); }

  function getItems(){
    // Return plain copies — callers shouldn't be able to mutate internal state directly.
    return items.map(it => ({ ...it }));
  }

  function findIndex(key){
    return items.findIndex(it => it.key === key);
  }

  function addItem(key, qty){
    qty = Math.max(1, Math.floor(qty) || 1);
    const i = findIndex(key);
    if (i >= 0) items[i].qty += qty;
    else items.push({ key, qty });
    persist();
  }

  function setQty(key, qty){
    qty = Math.floor(qty) || 0;
    const i = findIndex(key);
    if (i < 0) return;
    if (qty <= 0) items.splice(i, 1);
    else items[i].qty = qty;
    persist();
  }

  function removeItem(key){
    const i = findIndex(key);
    if (i < 0) return;
    items.splice(i, 1);
    persist();
  }

  function clear(){
    items = [];
    persist();
  }

  function getCount(){
    return items.reduce((sum, it) => sum + it.qty, 0);
  }

  function subscribe(fn){
    listeners.add(fn);
    return () => listeners.delete(fn);
  }

  // Keep multiple tabs in sync — if the cart changes in another tab,
  // pick that up here too.
  window.addEventListener('storage', (e)=>{
    if (e.key === STORAGE_KEY) {
      items = load();
      notify();
    }
  });

  window.RowenCart = { getItems, addItem, setQty, removeItem, clear, getCount, subscribe };
})();
