#include "ble_sched.h"

#include <string.h>

#include "alarm.h"
#include "ble.h"
#include "ble_gap.h"
#include "ble_gatts.h"
#include "board.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_sdh_ble.h"
#include "sched_codec.h"
#include "sched_store.h"

#define DEVICE_NAME "ShakeClock"
#define SERVICE_UUID16 0x0001
#define CHAR_UUID16 0x0002
#define ADV_INTERVAL 160u /* 0.625 ms units = 100 ms */
#define ADV_DURATION (BLE_SCHED_ADV_MS / 10u) /* 10 ms units */
#define GATT_MTU 247u
#define ERR_BUSY (BLE_GATT_STATUS_ATTERR_APP_BEGIN + 0x10)

/* e1430d6d-6e9b-4b29-ba99-6f30cbe203d0, little-endian; bytes 12-13 are the 16-bit alias. */
static const ble_uuid128_t s_base = { { 0xd0, 0x03, 0xe2, 0xcb, 0x30, 0x6f, 0x99, 0xba,
                                         0x29, 0x4b, 0x9b, 0x6e, 0x6d, 0x0d, 0x43, 0xe1 } };

NRF_BLE_GATT_DEF(s_gatt);
NRF_BLE_QWR_DEF(s_qwr);

static uint8_t s_uuid_type;
static ble_gatts_char_handles_t s_char;
static uint8_t s_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
static uint8_t s_adv_buf[31];
static uint8_t s_scan_buf[18];
static uint8_t s_qwr_mem[256];
static ble_gap_adv_data_t s_adv_data;

/* Written by the SoftDevice observer (interrupt context), read by ble_sched_poll. */
static volatile uint16_t s_conn = BLE_CONN_HANDLE_INVALID;
static volatile bool s_adv;
static volatile bool s_ev_connect, s_ev_disconnect, s_ev_adv_stop;
static volatile uint16_t s_ev_reject; /* last rejected status, 0 = none */
static volatile bool s_stage_ready;
static alarm_slot s_stage[ALARM_DAYS][ALARM_SLOTS]; /* valid while s_stage_ready is set */
static alarm_slot s_pend[ALARM_DAYS][ALARM_SLOTS];  /* decoded by AUTH_REQUEST, promoted at EXECUTE_WRITE */

static bool s_dropping;
static bool s_wrote;
static uint32_t s_conn_ms, s_write_ms;

static uint16_t status_for(sched_status s)
{
    return s == SCHED_ERR_LENGTH ? BLE_GATT_STATUS_ATTERR_INVALID_ATT_VAL_LENGTH
                                 : (uint16_t)(BLE_GATT_STATUS_ATTERR_APP_BEGIN + (uint16_t)s);
}

/* Validates one complete value into out. Rejects while the previous accepted write is not yet applied. */
static uint16_t validate(const uint8_t *data, size_t len, alarm_slot out[ALARM_DAYS][ALARM_SLOTS])
{
    if (s_stage_ready)
    {
        return ERR_BUSY;
    }
    sched_status st = sched_decode(data, len, out);
    return st == SCHED_OK ? BLE_GATT_STATUS_SUCCESS : status_for(st);
}

static void reject(uint16_t status)
{
    if (status != BLE_GATT_STATUS_SUCCESS)
    {
        s_ev_reject = status;
    }
}

static uint16_t qwr_cb(nrf_ble_qwr_t *p_qwr, nrf_ble_qwr_evt_t *p_evt)
{
    if (p_evt->attr_handle != s_char.value_handle)
    {
        return BLE_GATT_STATUS_SUCCESS;
    }
    if (p_evt->evt_type == NRF_BLE_QWR_EVT_EXECUTE_WRITE)
    {
        memcpy(s_stage, s_pend, sizeof s_stage);
        s_stage_ready = true;
        return BLE_GATT_STATUS_SUCCESS;
    }
    uint8_t buf[SCHED_LEN + 8];
    uint16_t len = sizeof buf;
    if (nrf_ble_qwr_value_get(p_qwr, p_evt->attr_handle, buf, &len) != NRF_SUCCESS)
    {
        reject(BLE_GATT_STATUS_ATTERR_UNLIKELY_ERROR);
        return BLE_GATT_STATUS_ATTERR_UNLIKELY_ERROR;
    }
    uint16_t st = validate(buf, len, s_pend);
    reject(st);
    return st;
}

static void on_authorize(const ble_gatts_evt_t *g)
{
    const ble_gatts_evt_rw_authorize_request_t *rq = &g->params.authorize_request;
    ble_gatts_rw_authorize_reply_params_t rp;
    memset(&rp, 0, sizeof rp);
    if (rq->type == BLE_GATTS_AUTHORIZE_TYPE_READ)
    {
        if (rq->request.read.handle != s_char.value_handle)
        {
            return;
        }
        /* The characteristic always mirrors the live table (UART edits included).
         * ponytail: read from interrupt context without a lock; a UART edit mid-read
         * can tear one slot. Upgrade: copy under a critical section. */
        static uint8_t enc[SCHED_LEN];
        alarm_slot t[ALARM_DAYS][ALARM_SLOTS];
        alarm_table_get(t);
        sched_encode((const alarm_slot(*)[ALARM_SLOTS])t, enc);
        rp.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
        rp.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;
        rp.params.read.update = 1;
        rp.params.read.len = SCHED_LEN;
        rp.params.read.p_data = enc;
        (void)sd_ble_gatts_rw_authorize_reply(g->conn_handle, &rp);
        return;
    }
    if (rq->request.write.handle != s_char.value_handle || rq->request.write.op != BLE_GATTS_OP_WRITE_REQ)
    {
        return; /* queued writes: nrf_ble_qwr replies */
    }
    uint16_t st = validate(rq->request.write.data, rq->request.write.len, s_pend);
    reject(st);
    if (st == BLE_GATT_STATUS_SUCCESS)
    {
        memcpy(s_stage, s_pend, sizeof s_stage);
        s_stage_ready = true;
    }
    rp.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
    rp.params.write.gatt_status = st;
    rp.params.write.update = st == BLE_GATT_STATUS_SUCCESS;
    rp.params.write.len = rq->request.write.len;
    rp.params.write.p_data = rq->request.write.data;
    (void)sd_ble_gatts_rw_authorize_reply(g->conn_handle, &rp);
}

static void on_ble_evt(ble_evt_t const *e, void *ctx)
{
    (void)ctx;
    switch (e->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        s_conn = e->evt.gap_evt.conn_handle;
        s_adv = false;
        s_ev_connect = true;
        (void)nrf_ble_qwr_conn_handle_assign(&s_qwr, s_conn);
        break;
    case BLE_GAP_EVT_DISCONNECTED:
        s_conn = BLE_CONN_HANDLE_INVALID;
        s_ev_disconnect = true;
        break;
    case BLE_GAP_EVT_ADV_SET_TERMINATED:
        s_adv = false;
        s_ev_adv_stop = true;
        break;
    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    {
        ble_gap_phys_t phys = { BLE_GAP_PHY_AUTO, BLE_GAP_PHY_AUTO };
        (void)sd_ble_gap_phy_update(e->evt.gap_evt.conn_handle, &phys);
        break;
    }
    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        (void)sd_ble_gap_sec_params_reply(e->evt.gap_evt.conn_handle,
                                          BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
        break;
    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        (void)sd_ble_gatts_sys_attr_set(e->evt.gatts_evt.conn_handle, NULL, 0, 0);
        break;
    case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        on_authorize(&e->evt.gatts_evt);
        break;
    default:
        break;
    }
}
NRF_SDH_BLE_OBSERVER(s_ble_obs, 3, on_ble_evt, NULL);

static void qwr_error(uint32_t e)
{
    (void)e;
}

bool ble_sched_init(void)
{
    uint32_t e;
    ble_gap_conn_sec_mode_t open;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&open);
    e = sd_ble_gap_device_name_set(&open, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
    ble_gap_conn_params_t pp = { 80, 160, 0, 400 }; /* 100-200 ms, latency 0, 4 s timeout */
    e |= sd_ble_gap_ppcp_set(&pp);
    e |= nrf_ble_gatt_init(&s_gatt, NULL);
    e |= nrf_ble_gatt_att_mtu_periph_set(&s_gatt, GATT_MTU);

    nrf_ble_qwr_init_t qi;
    memset(&qi, 0, sizeof qi);
    qi.error_handler = qwr_error;
    qi.mem_buffer.p_mem = s_qwr_mem;
    qi.mem_buffer.len = sizeof s_qwr_mem;
    qi.callback = qwr_cb;
    e |= nrf_ble_qwr_init(&s_qwr, &qi);

    e |= sd_ble_uuid_vs_add(&s_base, &s_uuid_type);
    ble_uuid_t su = { SERVICE_UUID16, s_uuid_type };
    uint16_t svc;
    e |= sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &su, &svc);

    ble_gatts_char_md_t cm;
    memset(&cm, 0, sizeof cm);
    cm.char_props.read = 1;
    cm.char_props.write = 1;
    ble_gatts_attr_md_t am;
    memset(&am, 0, sizeof am);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&am.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&am.write_perm);
    am.vloc = BLE_GATTS_VLOC_STACK;
    am.rd_auth = 1;
    am.wr_auth = 1;
    am.vlen = 1;
    ble_uuid_t cu = { CHAR_UUID16, s_uuid_type };
    static uint8_t init_val[SCHED_LEN];
    ble_gatts_attr_t at;
    memset(&at, 0, sizeof at);
    at.p_uuid = &cu;
    at.p_attr_md = &am;
    at.init_len = SCHED_LEN;
    at.max_len = SCHED_LEN;
    at.p_value = init_val;
    e |= sd_ble_gatts_characteristic_add(svc, &cm, &at, &s_char);
    e |= nrf_ble_qwr_attr_register(&s_qwr, s_char.value_handle);

    /* Adv: flags + complete name. Scan response: 128-bit service UUID. */
    uint8_t n = 0;
    s_adv_buf[n++] = 2;
    s_adv_buf[n++] = BLE_GAP_AD_TYPE_FLAGS;
    s_adv_buf[n++] = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    s_adv_buf[n++] = (uint8_t)(strlen(DEVICE_NAME) + 1);
    s_adv_buf[n++] = BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME;
    memcpy(&s_adv_buf[n], DEVICE_NAME, strlen(DEVICE_NAME));
    n += (uint8_t)strlen(DEVICE_NAME);
    s_adv_data.adv_data.p_data = s_adv_buf;
    s_adv_data.adv_data.len = n;
    s_scan_buf[0] = 17;
    s_scan_buf[1] = BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE;
    memcpy(&s_scan_buf[2], s_base.uuid128, 16);
    s_scan_buf[2 + 12] = SERVICE_UUID16 & 0xff;
    s_scan_buf[2 + 13] = SERVICE_UUID16 >> 8;
    s_adv_data.scan_rsp_data.p_data = s_scan_buf;
    s_adv_data.scan_rsp_data.len = sizeof s_scan_buf;

    if (e != NRF_SUCCESS)
    {
        con_printf("ble: init err %lu\n", (unsigned long)e);
    }
    return e == NRF_SUCCESS;
}

bool ble_sched_adv_start(void)
{
    if (ble_sched_busy())
    {
        return false;
    }
    ble_gap_adv_params_t p;
    memset(&p, 0, sizeof p);
    p.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    p.interval = ADV_INTERVAL;
    p.duration = ADV_DURATION;
    p.primary_phy = BLE_GAP_PHY_1MBPS;
    p.filter_policy = BLE_GAP_ADV_FP_ANY;
    uint32_t e = sd_ble_gap_adv_set_configure(&s_adv_handle, &s_adv_data, &p);
    if (e == NRF_SUCCESS)
    {
        e = sd_ble_gap_adv_start(s_adv_handle, BOARD_BLE_CONN_CFG_TAG);
    }
    if (e != NRF_SUCCESS)
    {
        con_printf("ble: adv err %lu\n", (unsigned long)e);
        return false;
    }
    s_adv = true;
    con_printf("ble: adv start\n");
    return true;
}

void ble_sched_stop(void)
{
    if (s_adv)
    {
        (void)sd_ble_gap_adv_stop(s_adv_handle);
        s_adv = false;
        s_ev_adv_stop = true;
    }
    if (s_conn != BLE_CONN_HANDLE_INVALID && !s_dropping)
    {
        s_dropping = sd_ble_gap_disconnect(s_conn, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION) == NRF_SUCCESS;
    }
}

bool ble_sched_busy(void)
{
    return s_adv || s_conn != BLE_CONN_HANDLE_INVALID;
}

static bool deadline(uint32_t *out)
{
    if (s_conn == BLE_CONN_HANDLE_INVALID || s_dropping)
    {
        return false;
    }
    *out = s_wrote ? s_write_ms + BLE_SCHED_WRITE_MS : s_conn_ms + BLE_SCHED_IDLE_MS;
    return true;
}

bool ble_sched_next_ms(uint32_t *out)
{
    return deadline(out);
}

bool ble_sched_poll(uint32_t now)
{
    bool applied = false;
    if (s_ev_connect)
    {
        s_ev_connect = false;
        s_conn_ms = now;
        s_wrote = false;
        s_dropping = false;
        con_printf("ble: connect\n");
    }
    if (s_ev_adv_stop)
    {
        s_ev_adv_stop = false;
        con_printf("ble: adv stop\n");
    }
    uint16_t rj = s_ev_reject;
    if (rj)
    {
        s_ev_reject = 0;
        con_printf("ble: write rejected 0x%02x\n", rj);
    }
    if (s_stage_ready)
    {
        bool ok = alarm_table_replace((const alarm_slot(*)[ALARM_SLOTS])s_stage);
        s_stage_ready = false;
        s_wrote = true;
        s_write_ms = now;
        if (!ok)
        {
            con_printf("ble: write rejected (table)\n");
        }
        else if (!sched_store_save())
        {
            con_printf("ble: save FAIL\n");
        }
        else
        {
            con_printf("ble: write ok\n");
            applied = true;
        }
    }
    if (s_ev_disconnect)
    {
        s_ev_disconnect = false;
        s_dropping = false;
        con_printf("ble: disconnect\n");
    }
    uint32_t d;
    if (deadline(&d) && (int32_t)(now - d) >= 0)
    {
        s_dropping = sd_ble_gap_disconnect(s_conn, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION) == NRF_SUCCESS;
    }
    return applied;
}
