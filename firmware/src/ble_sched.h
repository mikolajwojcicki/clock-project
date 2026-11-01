#ifndef BLE_SCHED_H
#define BLE_SCHED_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Alarm schedule over BLE: one GATT service, one characteristic (SCHED_LEN bytes, read and write).
 * Advertising is off until ble_sched_adv_start(). No pairing. Needs the SoftDevice enabled.
 * UUIDs: base e143xxxx-6e9b-4b29-ba99-6f30cbe203d0, service xxxx = 0x0001, characteristic 0x0002.
 */

#define BLE_SCHED_ADV_MS 60000u
#define BLE_SCHED_IDLE_MS 60000u  /**< disconnect this long after connect with no write */
#define BLE_SCHED_WRITE_MS 30000u /**< disconnect this long after the last write */

/** GAP name, GATT, QWR, service. Returns false (and prints the error) on failure. */
bool ble_sched_init(void);

/** Starts the advertising window (BLE_SCHED_ADV_MS). No-op while advertising or connected. */
bool ble_sched_adv_start(void);

/** Ends advertising and drops any link. Safe to call at any time. */
void ble_sched_stop(void);

/** True while advertising or connected. */
bool ble_sched_busy(void);

/**
 * Main-loop step: prints link events, applies an accepted write (table replace, flash save),
 * and enforces the link timeouts. Returns true when a write was applied on this call.
 */
bool ble_sched_poll(uint32_t now_ms);

/** Earliest time ble_sched_poll has timer work, if any (board_ms() time). */
bool ble_sched_next_ms(uint32_t *out);

#endif
