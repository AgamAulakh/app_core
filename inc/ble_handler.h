#include <zephyr/bluetooth/uuid.h>

#define BT_UUID_EMDS_VAL 0xABCD
#define BT_UUID_EMDS \
	BT_UUID_DECLARE_16(BT_UUID_EMDS_VAL)

int init_ble_handler(void);