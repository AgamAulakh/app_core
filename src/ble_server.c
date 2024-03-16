#include <stddef.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <ble_handler.h>


#define LOG_LEVEL CONFIG_BT_HRS_LOG_LEVEL
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(emds);

static uint8_t emds_blsc;


static void emdsc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	LOG_INF("EMDS notifications %s", notif_enabled ? "enabled" : "disabled");
}

/* EEG mTBI Detection Service Declaration */
BT_GATT_SERVICE_DEFINE(emds_svc,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_EMDS),
	BT_GATT_CHARACTERISTIC(BT_UUID_HRS_MEASUREMENT, BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CCC(emdsc_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE));

static int emds_init(void)
{
	emds_blsc = 0x01;

	return 0;
}

int bt_emds_notify(uint16_t heartrate)
{
	int err;
	static uint8_t hrm[2];

	hrm[0] = 0x06;
	hrm[1] = heartrate;

	err = bt_gatt_notify(NULL, &emds_svc.attrs[1], &hrm, sizeof(hrm));

	return err == -ENOTCONN ? 0 : err;
}

uint8_t bt_emds_write(uint32_t emd_result) {
    
    //bt_gatt_write();
	return 0;
}

SYS_INIT(emds_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);