#include "sched_store.h"

#include "alarm.h"
#include "board.h"
#include "nrf_fstorage.h"
#include "nrf_fstorage_sd.h"
#include "nrf_soc.h"
#include "sched_codec.h"

#define SCHED_PAGE 0x7F000u
#define SCHED_PAGE_SIZE 4096u

static void fs_evt(nrf_fstorage_evt_t *e)
{
    (void)e;
}

NRF_FSTORAGE_DEF(nrf_fstorage_t s_fs) = {
    .evt_handler = fs_evt,
    .start_addr = SCHED_PAGE,
    .end_addr = SCHED_PAGE + SCHED_PAGE_SIZE,
};

static void wait_idle(void)
{
    while (nrf_fstorage_is_busy(&s_fs))
    {
        (void)sd_app_evt_wait();
    }
}

bool sched_store_init(void)
{
    ret_code_t ie = nrf_fstorage_init(&s_fs, &nrf_fstorage_sd, NULL);
    if (ie != NRF_SUCCESS)
    {
        con_printf("fs init err %lu\n", (unsigned long)ie);
        return false;
    }
    alarm_slot t[ALARM_DAYS][ALARM_SLOTS];
    /* Flash is memory-mapped: read the record in place. */
    if (sched_record_unpack((const uint8_t *)SCHED_PAGE, t) != SCHED_OK)
    {
        return false;
    }
    return alarm_table_replace((const alarm_slot(*)[ALARM_SLOTS])t);
}

bool sched_store_save(void)
{
    alarm_slot t[ALARM_DAYS][ALARM_SLOTS];
    uint8_t rec[SCHED_RECORD_LEN];
    alarm_table_get(t);
    sched_record_pack((const alarm_slot(*)[ALARM_SLOTS])t, rec);
    ret_code_t e = nrf_fstorage_erase(&s_fs, SCHED_PAGE, 1, NULL);
    if (e != NRF_SUCCESS)
    {
        con_printf("erase err %lu\n", (unsigned long)e);
        return false;
    }
    wait_idle();
    e = nrf_fstorage_write(&s_fs, SCHED_PAGE, rec, sizeof rec, NULL);
    if (e != NRF_SUCCESS)
    {
        con_printf("write err %lu\n", (unsigned long)e);
        return false;
    }
    wait_idle();
    return true;
}
