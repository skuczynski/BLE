#ifndef BLE_H
#define BLE_H

class ble
{
public:
	void list_devices();
private:
	static int print_device_info(int s, int dev_id, long arg);
	static unsigned dev_count;
};

#endif
