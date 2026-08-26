/* Cadence Pave - phone side: settings, weather and sun times. */

var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clayPreview = require('./preview');
var clay = new Clay(clayConfig, clayPreview, { autoHandleEvents: false });

/* ------------------------------------------------------------- helpers */

/* WMO weather code -> the four sky glyphs the watch can draw */
function condOf(code) {
  if (code === 0) return 0;                          /* clear          */
  if (code === 1 || code === 2) return 1;            /* partly cloudy  */
  if (code === 3 || code === 45 || code === 48) return 2;  /* overcast */
  return 3;                                          /* anything wet   */
}

/* "2026-08-24T06:14" -> minutes since local midnight */
function minutesOf(iso) {
  if (!iso) return null;
  var t = iso.split('T')[1];
  if (!t) return null;
  var p = t.split(':');
  return parseInt(p[0], 10) * 60 + parseInt(p[1], 10);
}

function units() {
  var v = localStorage.getItem('UNITS');
  return (v === '1' || v === 1) ? 'celsius' : 'fahrenheit';
}

/* Minutes east of UTC for a named zone, right now. Uses the runtime's own zone
   database, so daylight saving is whatever the zone actually observes today. */
function zoneOffset(zone, date) {
  var dtf = new Intl.DateTimeFormat('en-US', {
    timeZone: zone, hour12: false,
    year: 'numeric', month: '2-digit', day: '2-digit',
    hour: '2-digit', minute: '2-digit', second: '2-digit'
  });
  var p = {};
  dtf.formatToParts(date).forEach(function (x) { p[x.type] = x.value; });
  var asUTC = Date.UTC(+p.year, +p.month - 1, +p.day, (+p.hour) % 24, +p.minute, +p.second);
  return Math.round((asUTC - date.getTime()) / 60000);
}

/* Fallback standard offsets, used only if the runtime has no zone database.
   These do not follow DST; the Intl path above is the one that normally runs. */
var FALLBACK = {
  'Pacific/Honolulu': -600, 'America/Anchorage': -540, 'America/Los_Angeles': -480,
  'America/Vancouver': -480, 'America/Phoenix': -420, 'America/Denver': -420,
  'America/Chicago': -360, 'America/Mexico_City': -360, 'America/New_York': -300,
  'America/Toronto': -300, 'America/Bogota': -300, 'America/Halifax': -240,
  'America/St_Johns': -210, 'America/Sao_Paulo': -180, 'UTC': 0,
  'Atlantic/Reykjavik': 0, 'Europe/London': 0, 'Europe/Dublin': 0, 'Europe/Lisbon': 0,
  'Europe/Paris': 60, 'Europe/Brussels': 60, 'Europe/Amsterdam': 60, 'Europe/Madrid': 60,
  'Europe/Rome': 60, 'Europe/Berlin': 60, 'Europe/Zurich': 60, 'Europe/Oslo': 60,
  'Europe/Athens': 120, 'Europe/Helsinki': 120, 'Africa/Cairo': 120,
  'Africa/Johannesburg': 120, 'Europe/Moscow': 180, 'Africa/Nairobi': 180,
  'Asia/Dubai': 240, 'Asia/Karachi': 300, 'Asia/Kolkata': 330, 'Asia/Kathmandu': 345,
  'Asia/Dhaka': 360, 'Asia/Bangkok': 420, 'Asia/Singapore': 480, 'Asia/Hong_Kong': 480,
  'Asia/Shanghai': 480, 'Australia/Perth': 480, 'Asia/Tokyo': 540, 'Asia/Seoul': 540,
  'Australia/Adelaide': 570, 'Australia/Brisbane': 600, 'Australia/Sydney': 600,
  'Pacific/Auckland': 720
};

function zoneName() {
  return localStorage.getItem('TZ_ZONE') || 'Europe/Paris';
}

/* minutes to add to LOCAL time to get the second zone's time */
function tzDelta() {
  var zone = zoneName();
  var now = new Date();
  var local = -now.getTimezoneOffset();
  var there;
  try {
    there = zoneOffset(zone, now);
    if (typeof there !== 'number' || isNaN(there)) throw new Error('bad offset');
  } catch (e) {
    console.log('Intl zone lookup unavailable, using fallback: ' + e.message);
    there = (zone in FALLBACK) ? FALLBACK[zone] : 0;
  }
  return there - local;
}

/* "America/Halifax" -> "HALIFAX", for when the label is left blank */
function zoneLabel() {
  var set = localStorage.getItem('TZ_LABEL');
  if (set && set.trim()) return set.trim().toUpperCase().substring(0, 7);
  var city = zoneName().split('/').pop().replace(/_/g, ' ');
  return city.toUpperCase().substring(0, 7);
}

function send(msg) {
  Pebble.sendAppMessage(msg,
    function () {},
    function (e) { console.log('send failed: ' + JSON.stringify(e)); });
}

/* ------------------------------------------------------------- weather */
function fetchWeather() {
  navigator.geolocation.getCurrentPosition(function (pos) {
    var url = 'https://api.open-meteo.com/v1/forecast'
      + '?latitude=' + pos.coords.latitude.toFixed(3)
      + '&longitude=' + pos.coords.longitude.toFixed(3)
      + '&current=temperature_2m,weather_code'
      + '&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset'
      + '&timezone=auto&forecast_days=1'
      + '&temperature_unit=' + units();

    var req = new XMLHttpRequest();
    req.open('GET', url, true);
    req.onload = function () {
      if (req.status !== 200) { console.log('weather http ' + req.status); return; }
      var d;
      try { d = JSON.parse(req.responseText); }
      catch (e) { console.log('weather parse failed'); return; }

      var msg = {};
      if (d.current) {
        msg.TEMP = Math.round(d.current.temperature_2m);
        msg.COND = condOf(d.current.weather_code);
      }
      if (d.daily) {
        msg.HI = Math.round(d.daily.temperature_2m_max[0]);
        msg.LO = Math.round(d.daily.temperature_2m_min[0]);
        var sr = minutesOf(d.daily.sunrise[0]);
        var ss = minutesOf(d.daily.sunset[0]);
        if (sr !== null) msg.SUNRISE = sr;
        if (ss !== null) msg.SUNSET = ss;
      }
      msg.TZ_OFFSET = tzDelta();
      msg.TZ_LABEL = zoneLabel();
      send(msg);
    };
    req.onerror = function () { console.log('weather request failed'); };
    req.send();
  }, function (err) {
    console.log('location failed: ' + err.message);
  }, { timeout: 15000, maximumAge: 600000 });
}

/* ------------------------------------------------------------- lifecycle */
Pebble.addEventListener('ready', function () {
  send({ TZ_OFFSET: tzDelta(), TZ_LABEL: zoneLabel() });
  fetchWeather();
});

/* the watch pokes us every 30 minutes */
Pebble.addEventListener('appmessage', function () {
  fetchWeather();
});

Pebble.addEventListener('showConfiguration', function () {
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function (e) {
  if (!e || !e.response) return;

  var dict = clay.getSettings(e.response, false);
  var out = {};

  /* Clay hands back its own keyed dict; unwrap to plain message keys and
     remember the values we need on the phone side. */
  Object.keys(dict).forEach(function (k) {
    var v = dict[k];
    var val = (v && typeof v === 'object' && 'value' in v) ? v.value : v;
    localStorage.setItem(k, val);
    if (k === 'TZ_ZONE' || k === 'TZ_LABEL') return;   /* handled below */
    out[k] = parseInt(val, 10);
  });

  out.TZ_OFFSET = tzDelta();
  out.TZ_LABEL = zoneLabel();
  send(out);
  fetchWeather();
});
