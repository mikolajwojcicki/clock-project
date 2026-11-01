// Run: nix-shell -p nodejs --run "node web/schedule.test.mjs"
import assert from 'node:assert/strict';
import { emptyTable, encode, decode, findDuplicates, LENGTH } from './schedule.js';

// Monday slot 1 07:00 enabled -> 01 A4 81, rest unused.
let t = emptyTable();
t[0][0] = { enabled: true, minute: 420 };
let b = encode(t);
assert.equal(b.length, LENGTH);
assert.deepEqual([...b.slice(0, 5)], [0x01, 0xa4, 0x81, 0x00, 0x00]);
assert.ok(b.slice(3).every((x) => x === 0));

// Round trip, including a disabled slot that keeps its time and the last minute of the day.
t[2][5] = { enabled: false, minute: 90 };
t[6][7] = { enabled: true, minute: 1439 };
assert.deepEqual(decode(encode(t)), t);
assert.deepEqual([...encode(decode(encode(t)))], [...encode(t)]);

// Duplicates: same day flagged, other days and disabled slots not.
t = emptyTable();
t[1][0] = { enabled: true, minute: 390 };
t[1][3] = { enabled: true, minute: 390 };
assert.deepEqual(findDuplicates(t), [{ day: 1, slots: [0, 3] }]);
t = emptyTable();
t[0][0] = { enabled: true, minute: 390 };
t[1][0] = { enabled: true, minute: 390 };
assert.deepEqual(findDuplicates(t), []);
t[1][1] = { enabled: false, minute: 390 };
assert.deepEqual(findDuplicates(t), []);

// Rejects malformed input.
b = encode(emptyTable());
assert.throws(() => decode(b.slice(0, 21)), /length/);
b[0] = 2;
assert.throws(() => decode(b), /version/);
b[0] = 1;
b[1] = 0xa1; b[2] = 0x05; // 0x05A1 = 1441
assert.throws(() => decode(b), /minute/);
assert.throws(() => encode(Object.assign(emptyTable(), { 0: [{ enabled: true, minute: 1440 }, ...emptyTable()[0].slice(1)] })), /range/);

console.log('schedule.test: ok');
