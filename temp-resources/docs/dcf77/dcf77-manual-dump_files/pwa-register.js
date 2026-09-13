(function () {
  if (location.hostname !== ('manuals' + '.plus')) return;
  if (window.manualsPwaRegistered || !('serviceWorker' in navigator) || window.top !== window.self) return;
  window.manualsPwaRegistered = true;
  if (/^\/(?:wp-admin|import-admin)(?:\/|$)/.test(location.pathname)) return;
  function register() {
    navigator.serviceWorker.register('/sw.js', {scope: '/', updateViaCache: 'none'})
      .catch(function () {});
  }
  // Let page resources and ads initialize before service-worker installation.
  function afterLoad() {
    window.setTimeout(function () {
      if ('requestIdleCallback' in window) window.requestIdleCallback(register, {timeout: 5000});
      else register();
    }, 10000);
  }
  if (document.readyState === 'complete') afterLoad();
  else window.addEventListener('load', afterLoad, {once: true});
})();
