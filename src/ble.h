#ifndef BLE_H
#define BLE_H

#include <cstdint>
#include <cstddef>

class ble
{
public:
	void list_devices();
	void scan_le_devices(int dev_id);
private:
	static int print_device_info(int descriptor, int dev_id, long arg);
	static int print_advertising_devices(int device_handle, uint8_t filter_type);
	static void sigint_handler(int sig);
	static void eir_get_field(uint8_t *eir, size_t eir_length, char *buf, size_t buf_length, uint8_t type);
	static unsigned dev_count;
	static volatile int signal_received;
};

#endif
