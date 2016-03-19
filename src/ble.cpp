#include "ble.h"

#include <cstdio>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <sys/ioctl.h>

unsigned ble::dev_count = 0;

void ble::list_devices()
{
	hci_for_each_dev(HCI_UP, ble::print_device_info, 0);

	printf("Total: %d BT devices\n", dev_count);
}

int ble::print_device_info(int descriptor, int dev_id, long arg)
{
	struct hci_dev_info dev_info = { .dev_id = (uint16_t)dev_id };
	char mac_addr[18];

	if (ioctl(descriptor, HCIGETDEVINFO, (void *)&dev_info))
	{
		printf("Problem occured, when retrieved device info(id: %d)\n", dev_id);
		return 0;
	}

	ba2str(&dev_info.bdaddr, mac_addr);
	printf("id: %u, dev_name: %s, MAC: %s\n", dev_info.dev_id, dev_info.name, mac_addr);
	++dev_count;
	return 0;
}
