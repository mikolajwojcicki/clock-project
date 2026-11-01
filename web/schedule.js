// Schedule codec shared by the page and the Node check. No DOM, no Bluetooth.
// Wire format: see openspec/specs/ble-schedule-service. Firmware: firmware/src/ble_sched.[ch].

// Firmware registers base e143xxxx-6e9b-4b29-ba99-6f30cbe203d0 and replaces xxxx
// with 0x0001 (service) and 0x0002 (characteristic).
export const SERVICE_UUID = 'e1430001-6e9b-4b29-ba99-6f30cbe203d0';
export const CHAR_UUID = 'e1430002-6e9b-4b29-ba99-6f30cbe203d0';
export const DEVICE_NAME = 'ShakeClock';

export const DAYS = 7;
export const SLOTS = 8;
export const LENGTH = 1 + DAYS * SLOTS * 2; // 113
export const VERSION = 1;
const ENABLE = 0x8000;
const MINUTE_MASK = 0x07ff;

// Table: days[7][8] of {enabled: bool, minute: int | null}; null minute = unused slot.
export function emptyTable() {
  return Array.from({ length: DAYS }, () =>
    Array.from({ length: SLOTS }, () => ({ enabled: false, minute: null })));
}

export function encode(days) {
  const out = new Uint8Array(LENGTH);
  out[0] = VERSION;
  for (let d = 0; d < DAYS; d++) {
    for (let s = 0; s < SLOTS; s++) {
      const { enabled, minute } = days[d][s];
      let v = 0;
      if (minute !== null) {
        if (!Number.isInteger(minute) || minute < 0 || minute > 1439) {
          throw new RangeError(`minute out of range: ${minute}`);
        }
        v = minute | (enabled ? ENABLE : 0);
      }
      const i = 1 + (d * SLOTS + s) * 2;
      out[i] = v & 0xff;
      out[i + 1] = v >> 8;
    }
  }
  return out;
}

export function decode(bytes) {
  const b = bytes instanceof Uint8Array ? bytes : new Uint8Array(bytes.buffer ?? bytes);
  if (b.length !== LENGTH) throw new Error(`bad length ${b.length}, expected ${LENGTH}`);
  if (b[0] !== VERSION) throw new Error(`bad version ${b[0]}, expected ${VERSION}`);
  const days = emptyTable();
  for (let d = 0; d < DAYS; d++) {
    for (let s = 0; s < SLOTS; s++) {
      const i = 1 + (d * SLOTS + s) * 2;
      const v = b[i] | (b[i + 1] << 8);
      if (v === 0) continue;
      const minute = v & MINUTE_MASK;
      if (minute > 1439) throw new Error(`bad minute ${minute} at day ${d} slot ${s}`);
      days[d][s] = { enabled: (v & ENABLE) !== 0, minute };
    }
  }
  return days;
}

// Firmware rule: two enabled slots with the same minute on one weekday.
// Returns [{day, slots: [s, ...]}].
export function findDuplicates(days) {
  const found = [];
  for (let d = 0; d < DAYS; d++) {
    const byMinute = new Map();
    days[d].forEach((slot, s) => {
      if (!slot.enabled || slot.minute === null) return;
      if (!byMinute.has(slot.minute)) byMinute.set(slot.minute, []);
      byMinute.get(slot.minute).push(s);
    });
    for (const slots of byMinute.values()) if (slots.length > 1) found.push({ day: d, slots });
  }
  return found;
}
