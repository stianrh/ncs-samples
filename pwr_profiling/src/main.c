#include <zephyr.h>
#include <sys/printk.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <logging/log.h>

LOG_MODULE_REGISTER(MyApp,LOG_LEVEL_DBG);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static K_SEM_DEFINE(ble_init_ok, 0, 1);

/* Data to be used in advertisement packets */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

/* Set Scan Response data */
static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection failed (err %u)", err);
        return;
    }
    LOG_INF("Connection established!");
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected (reason %u)", reason);
}

static struct bt_conn_cb conn_callbacks = {
    .connected             = connected,        // new connection has been established
    .disconnected          = disconnected,     // connection has been disconnected
};

void bt_ready(int err)
{
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return;
    }
    LOG_INF("Bluetooth initialized");
    bt_conn_cb_register(&conn_callbacks);
    err = bt_le_adv_start(
        BT_LE_ADV_PARAM(
            BT_LE_ADV_OPT_CONNECTABLE, \
            CONFIG_APP_ADV_INT_MS / 0.625, \
            CONFIG_APP_ADV_INT_MS / 0.625, \
            NULL
        ),
        ad,
        ARRAY_SIZE(ad),
        sd,
        ARRAY_SIZE(sd)
    );
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return;
    }
    LOG_INF("Advertising successfully started");

    k_sem_give(&ble_init_ok);
}

static struct k_timer cpu_run;
static void cpu_run_fn(struct k_timer *work)
{
    k_busy_wait((CONFIG_APP_CPU_RUN_INTERVAL_MS * CONFIG_APP_CPU_RUN_DUTY_CYCLE/100) * 1000);
}

void main(void)
{
    int err;

    LOG_INF("Started\n");
    bt_enable(bt_ready);
    err = k_sem_take(&ble_init_ok, K_MSEC(500));
    if (!err)
    {
        LOG_INF("Bluetooth initialized\n");
    } else
    {
        LOG_ERR("BLE initialization did not complete in time\n");
    }

    /* Start CPU load timer */
    if(CONFIG_APP_CPU_RUN_DUTY_CYCLE != 0 && CONFIG_APP_CPU_RUN_INTERVAL_MS != 0){
        k_timer_init(&cpu_run, cpu_run_fn, NULL);
        k_timer_start(&cpu_run, K_MSEC(CONFIG_APP_CPU_RUN_INTERVAL_MS), K_MSEC(CONFIG_APP_CPU_RUN_INTERVAL_MS));
    }
}
