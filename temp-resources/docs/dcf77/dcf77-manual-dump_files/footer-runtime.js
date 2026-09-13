(function (w, d) {
  'use strict';
  if (w.ManualsFooter) return;
  var script = d.currentScript;
  var clientSrc = script.dataset.accountSrc;
  var login = 'https://manuals.plus/login/';
  var loading = null;
  function safeReturn(value) {
    if (typeof value !== 'string' || value.length > 4096 || /[\u0000-\u0020\u007f\\]/.test(value)) return '/account';
    var decoded = value;
    try {
      for (var i = 0; i < 3; i++) { decoded = decodeURIComponent(decoded); if (/[\u0000-\u001f\u007f\\]/.test(decoded)) return '/account'; }
      if (value.indexOf('//') === 0 || decoded.indexOf('//') === 0) return '/account';
      var url = new URL(value, 'https://manuals.plus');
      if (url.protocol !== 'https:' || !/^(?:[a-z0-9](?:[a-z0-9-]{0,61}[a-z0-9])?\.)*manuals\.plus$/i.test(url.hostname)
          || url.username || url.password || (url.port && url.port !== '443')
          || /^\/(?:login|logout|challenge-login|wp-login\.php)(?:[/.]|$)/i.test(url.pathname)) return '/account';
      return url.href;
    } catch (_) { return '/account'; }
  }
  function account() {
    if (w.ManualsAccount) return Promise.resolve(w.ManualsAccount);
    if (loading) return loading;
    loading = new Promise(function (resolve, reject) {
      var s = d.createElement('script'); s.src = clientSrc; s.defer = true;
      s.onload = function () { if (w.ManualsAccount) resolve(w.ManualsAccount); else { loading = null; reject(new Error('account_unavailable')); } };
      s.onerror = function () { loading = null; s.remove(); reject(new Error('account_unavailable')); };
      d.head.appendChild(s);
    });
    return loading;
  }
  function beginLogin(destination) {
    var form = d.createElement('form'); form.method = 'post'; form.action = 'https://manuals.plus/login/intent.php';
    var input = d.createElement('input'); input.type = 'hidden'; input.name = 'destination'; input.value = safeReturn(destination || w.location.href);
    form.appendChild(input); d.body.appendChild(form); form.submit();
  }
  function pendingSave() {
    var page = w.MANUALS_ACCOUNT_PAGE;
    if (!page || !page.resource_type || !page.resource_key) return;
    try { w.localStorage.setItem('manuals-account-pending-save', JSON.stringify({resource_type: page.resource_type, resource_key: page.resource_key, canonical_url: page.canonical_url || w.location.href, title: page.title || d.title, note: ''})); } catch (_) {}
  }
  function renderSave() {
    var page = w.MANUALS_ACCOUNT_PAGE;
    if (!page) return;
    var host = page.mountSelector ? d.querySelector(page.mountSelector) : d.querySelector('[data-manuals-save-anchor]');
    if (!host || host.querySelector('[data-manuals-save-button], [data-manuals-save-intent]')) return;
    var button = d.createElement('button'); button.type = 'button'; button.className = 'manuals-save-intent';
    button.dataset.manualsSaveIntent = ''; button.textContent = 'Sign in to save this manual'; host.appendChild(button);
  }
  function credential(response) {
    if (!response || !response.credential || !/^(fedcm|itp|user|user_1tap|user_2tap|btn|btn_confirm|btn_add_session|btn_confirm_add_session|itp_confirm)$/.test(response.select_by || '')) return;
    w.MANUALS_ACCOUNT_LOGIN_INTENT = true;
    account().then(function (a) { return a.loginWithCredential(response.credential, response.select_by); }).catch(function () {
      w.dispatchEvent(new CustomEvent('manualsAccount:loginError'));
    });
  }
  w.ManualsFooter = { beginLogin: beginLogin, loadAccount: account, safeReturn: safeReturn, credential: credential };
  d.addEventListener('click', function (event) {
    var target = event.target.closest && event.target.closest('[data-manuals-login], [data-manuals-save-intent]');
    if (!target || event.defaultPrevented || event.button !== 0 || event.metaKey || event.ctrlKey || event.shiftKey || event.altKey) return;
    // A real link still supports copy/new-tab/no-JS navigation with referrer fallback.
    if (target.matches('[data-manuals-login]') && target.href !== login) return;
    event.preventDefault();
    if (target.hasAttribute('data-manuals-save-intent')) pendingSave();
    beginLogin(w.location.href);
  });
  function start() {
    renderSave();
    var known = script.dataset.signedIn === 'true' || /(?:^|;\s*)manuals_account_active=1(?:;|$)/.test(d.cookie);
    if (known) {
      w.MANUALS_ACCOUNT_HAS_SESSION = true;
      account().catch(function () {});
    }
    if (w.manualsFooterCredential) { var response = w.manualsFooterCredential; delete w.manualsFooterCredential; credential(response); }
    if (w.location.hostname === ('manuals' + '.plus') && script.dataset.pwaSrc && w.top === w.self && 'serviceWorker' in navigator) {
      function loadPwa() {
        var pwa = d.createElement('script'); pwa.src = script.dataset.pwaSrc; pwa.defer = true; d.head.appendChild(pwa);
      }
      if (d.readyState === 'complete') loadPwa();
      else w.addEventListener('load', loadPwa, {once: true});
    }
    if (script.dataset.prefetchSrc) { var prefetch = d.createElement('script'); prefetch.type = 'module'; prefetch.src = script.dataset.prefetchSrc; d.head.appendChild(prefetch); }
  }
  w.addEventListener('manualsAccount:logout', function () {
    d.cookie = 'manuals_account_active=; Max-Age=0; Path=/; Domain=manuals.plus; Secure; SameSite=Lax';
    if (navigator.serviceWorker && navigator.serviceWorker.controller) navigator.serviceWorker.controller.postMessage({type: 'manuals-logout'});
  });
  if (d.readyState === 'loading') d.addEventListener('DOMContentLoaded', start, {once: true}); else start();
})(window, document);
