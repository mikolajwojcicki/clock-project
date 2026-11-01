#ifndef SCHED_STORE_H
#define SCHED_STORE_H

#include <stdbool.h>

/** Registers the fstorage instance (SoftDevice must be enabled) and loads the record into the alarm
 *  table. A missing or corrupt record leaves every slot disabled. Returns true if a record was loaded. */
bool sched_store_init(void);

/** Erases the schedule page and writes the current alarm table. Blocks until done (~100 ms). */
bool sched_store_save(void);

#endif
