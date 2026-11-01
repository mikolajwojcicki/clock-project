import {
  SERVICE_UUID, CHAR_UUID, DEVICE_NAME, DAYS, SLOTS,
  emptyTable, encode, decode, findDuplicates,
} from './schedule.js';

const DAY_NAMES = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday'];
const HINT = 'No ShakeClock found. Hold a clock button for 2 s to open the 60 s advertising window, then press Connect again.';

const $ = (id) => document.getElementById(id);
const connectBtn = $('connect');
const uploadBtn = $('upload');

let table = emptyTable();   // what the user sees
let baseline = encode(table); // last value known to be on the clock (or empty at start)
let device = null;
let characteristic = null;
let state = 'disconnected';

const minuteToStr = (m) => (m === null ? '' : `${String(Math.floor(m / 60)).padStart(2, '0')}:${String(m % 60).padStart(2, '0')}`);
const strToMinute = (s) => {
  const m = /^(\d{2}):(\d{2})$/.exec(s);
  return m ? Number(m[1]) * 60 + Number(m[2]) : null;
};
const sameBytes = (a, b) => a.length === b.length && a.every((x, i) => x === b[i]);
const isModified = () => !sameBytes(encode(table), baseline);

function say(text, kind = '') {
  const el = $('msg');
  el.textContent = text;
  el.className = kind;
}

function setState(s) {
  state = s;
  const el = $('state');
  el.textContent = s;
  el.className = s;
  render();
}

function render() {
  const dups = findDuplicates(table);
  const dupSet = new Set(dups.flatMap(({ day, slots }) => slots.map((s) => `${day}:${s}`)));
  const base = decode(baseline);
  const grid = $('grid');
  grid.replaceChildren();
  const head = grid.insertRow();
  head.insertCell().outerHTML = '<th></th>';
  for (let s = 0; s < SLOTS; s++) head.insertCell().outerHTML = `<th>Slot ${s + 1}</th>`;

  for (let d = 0; d < DAYS; d++) {
    const row = grid.insertRow();
    row.insertCell().outerHTML = `<th>${DAY_NAMES[d]}</th>`;
    for (let s = 0; s < SLOTS; s++) {
      const slot = table[d][s];
      const cell = row.insertCell();
      if (slot.minute === null) cell.className = 'unused';
      if (dupSet.has(`${d}:${s}`)) cell.classList.add('dup');
      if (slot.enabled !== base[d][s].enabled || slot.minute !== base[d][s].minute) cell.classList.add('changed');

      const on = document.createElement('input');
      on.type = 'checkbox';
      on.checked = slot.enabled;
      on.disabled = slot.minute === null;
      on.title = 'enabled';
      on.addEventListener('change', () => { slot.enabled = on.checked; edited(); });

      const time = document.createElement('input');
      time.type = 'time';
      time.value = minuteToStr(slot.minute);
      time.addEventListener('change', () => {
        const m = strToMinute(time.value);
        slot.minute = m;
        if (m === null) slot.enabled = false;       // cleared time = unused slot
        else if (base[d][s].minute === null) slot.enabled = true; // newly set slot starts enabled
        edited();
      });

      cell.append(on, document.createElement('br'), time);
    }
  }

  const modified = isModified();
  $('dirty').hidden = !modified;
  uploadBtn.disabled = state !== 'connected' || dups.length > 0;
  uploadBtn.title = dups.length ? 'Two enabled slots on one day have the same time' : '';
}

function edited() {
  say('');
  render();
}

if (!navigator.bluetooth) {
  connectBtn.disabled = true;
  say('This browser has no Web Bluetooth support. Use Chrome, Chromium or Edge on localhost or HTTPS.', 'err');
}

async function connect() {
  say('');
  setState('connecting');
  try {
    device = await navigator.bluetooth.requestDevice({
      filters: [{ name: DEVICE_NAME, services: [SERVICE_UUID] }],
      optionalServices: [SERVICE_UUID],
    });
  } catch (e) {
    setState('disconnected');
    say(e.name === 'NotFoundError' ? HINT : `Connect failed: ${e.message}`, e.name === 'NotFoundError' ? '' : 'err');
    return;
  }
  device.addEventListener('gattserverdisconnected', onDisconnected);
  try {
    const server = await device.gatt.connect();
    const service = await server.getPrimaryService(SERVICE_UUID);
    characteristic = await service.getCharacteristic(CHAR_UUID);
    const fromClock = new Uint8Array((await characteristic.readValue()).buffer.slice(0));
    const remote = decode(fromClock);
    if (!isModified() || confirm('The clock table differs from your unsaved edits.\n\nOK = load the clock table (edits lost)\nCancel = keep your edits')) {
      table = remote;
      say('Connected. Table loaded from the clock.', 'ok');
    } else {
      say('Connected. Your unsaved edits were kept; the clock table was not loaded.');
    }
    baseline = fromClock;
    setState('connected');
  } catch (e) {
    if (device.gatt.connected) device.gatt.disconnect();
    setState('disconnected');
    say(`Connect failed: ${e.message}`, 'err');
  }
}

function onDisconnected() {
  characteristic = null;
  setState('disconnected');
  say('Link dropped by the clock. Edits are kept. Hold a button for 2 s and connect again.');
}

async function upload() {
  if (!characteristic) return;
  const bytes = encode(table);
  try {
    await characteristic.writeValueWithResponse(bytes);
    const back = new Uint8Array((await characteristic.readValue()).buffer.slice(0));
    if (!sameBytes(back, bytes)) {
      say('Upload error: the clock returned a different table than was written.', 'err');
      return;
    }
    baseline = bytes;
    render();
    say('Saved to the clock. Read-back matches.', 'ok');
  } catch (e) {
    if (state === 'connected') say(`Upload error: ${e.message}`, 'err');
  }
}

connectBtn.addEventListener('click', connect);
uploadBtn.addEventListener('click', upload);
render();
