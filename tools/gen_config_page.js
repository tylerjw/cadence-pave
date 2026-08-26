/* Dev helper: run the built PKJS bundle with a stubbed Pebble object and
   capture the settings page HTML that Clay would open on the phone. */
const fs = require('fs');
const path = require('path');
const ROOT = path.join(__dirname, '..');
const listeners = {};
let captured = null;
global.window = global;
global.localStorage = { _d: {}, getItem(k) { return this._d[k] || null; },
                        setItem(k, v) { this._d[k] = String(v); } };
Object.defineProperty(global, 'navigator', {
  value: { geolocation: { getCurrentPosition() {} } }, writable: true, configurable: true });
global.XMLHttpRequest = function () { this.open = () => {}; this.send = () => {}; };
global.Pebble = {
  platform: 'ios',
  addEventListener: (ev, fn) => { (listeners[ev] = listeners[ev] || []).push(fn); },
  openURL: (u) => { captured = u; },
  sendAppMessage: () => {},
  getActiveWatchInfo: () => ({ platform: 'emery', model: 'emery', language: 'en', firmware: {} }),
  getAccountToken: () => 'acct', getWatchToken: () => 'watch',
};
require(path.join(ROOT, 'build/pebble-js-app.js'));
(listeners['ready'] || []).forEach(f => f());
(listeners['showConfiguration'] || []).forEach(f => f());
if (!captured) { console.error('no URL captured'); process.exit(1); }
const m = captured.match(/^data:text\/html(?:;[^,]*)?,([\s\S]*)$/);
const html = m ? decodeURIComponent(m[1]) : captured;
fs.writeFileSync(path.join(ROOT, 'build/settings-preview.html'), html);
console.log('wrote build/settings-preview.html', html.length, 'bytes');
