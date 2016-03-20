#include "ble.h"

#include <cstdio>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>

unsigned ble::dev_count = 0;

void ble::list_devices()
{
	hci_for_each_dev(HCI_UP, ble::print_device_info, 0);

	printf("Total: %d BT devices\n", dev_count);
}


void ble::scan_le_devices(int dev_id)
{
	int error_number, device_handle;
	uint8_t own_type = LE_PUBLIC_ADDRESS;
	uint8_t scan_type = 0x01;
	uint8_t filter_type = 0;
	uint8_t filter_policy = 0x00;
	uint16_t interval = htobs(0x0010);
	uint16_t window = htobs(0x0010);
	uint8_t filter_dup = 0x01;

	device_handle = hci_open_dev(dev_id);
	if(device_handle < 0) 
	{
		printf("Failed to open device");
		return;
	}

	error_number = hci_le_set_scan_parameters(device_handle, scan_type, interval, window,
						own_type, filter_policy, 10000);
	if(error_number < 0) 
	{
		printf("Failed to set scan parameters");
		return;
	}

	error_number = hci_le_set_scan_enable(device_handle, 0x01, filter_dup, 10000);
	if(error_number < 0)
	{
		printf("Failed to enable scan");
		return;
	}

	printf("Scanning for BLE devices:\n");

	error_number = ble::print_advertising_devices(device_handle, filter_type);
	if (error_number < 0)
	{
		printf("Failed to receive advertising events");
		return;
	}

	error_number = hci_le_set_scan_enable(device_handle, 0x00, filter_dup, 10000);
	if (error_number < 0)
	{
		printf("Failed to disable scan");
		return;
	}

	hci_close_dev(device_handle);
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

int ble::print_advertising_devices(int device_handle, uint8_t filter_type)
{
	unsigned char buffer[HCI_MAX_EVENT_SIZE], *ptr;
	hci_filter nf, of;
	struct sigaction sa;
	socklen_t olen;
	int len;

	olen = sizeof(of);
	if (getsockopt(device_handle, SOL_HCI, HCI_FILTER, &of, &olen) < 0) 
	{
		printf("Failed to get socket options\n");
		return -1;
	}

	hci_filter_clear(&nf);
	hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
	hci_filter_set_event(EVT_LE_META_EVENT, &nf);

	if (setsockopt(device_handle, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) 
	{
		printf("Failed set socket options\n");
		return -1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_NOCLDSTOP;
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa, NULL);

	while (1) 
	{
		evt_le_meta_event *meta;
		le_advertising_info *info;
		char mac_addr[18];
		int done = 0;

		while ((len = read(device_handle, buffer, sizeof(buffer))) < 0) 
		{
			if (errno == EINTR && signal_received == SIGINT) 
			{
				len = 0;
				done = 1;
				break;
			}

			//temporary unavailable
			//or
			//interrupted
			if (errno == EAGAIN || errno == EINTR)
				continue;
			
			done = 1;
			break;
		}
		if(done == 1)
			break;

		ptr = buffer + (1 + HCI_EVENT_HDR_SIZE); //HCI_EVENT_HDR_SIZE = 2
		len -= (1 + HCI_EVENT_HDR_SIZE);

		meta = (evt_le_meta_event *) ptr;

		if (meta->subevent != 0x02)
			break;

		info = (le_advertising_info *) (meta->data + 1);
		char name[30];

		memset(name, 0, sizeof(name));

		ba2str(&info->bdaddr, mac_addr);
		eir_get_field(info->data, info->length, name, sizeof(name) - 1, 0x08);//name_short

		int8_t RSSI = (int8_t)info->data[info->length];

		printf("---\nMAC: %s \nname: %s\nRSSI: %d\n", mac_addr, name, RSSI);
	}

	setsockopt(device_handle, SOL_HCI, HCI_FILTER, &of, sizeof(of));

	if (len < 0)
		return -1;

	return 0;
}

void ble::sigint_handler(int sig)
{
	signal_received = sig;
}

//Extended Inquiry Response
void ble::eir_get_field(uint8_t *eir, size_t eir_length, char *buf, size_t buf_length, uint8_t type)
{
	size_t offset;

	offset = 0;
	while (offset < eir_length) 
	{
		uint8_t field_len = eir[0];
		size_t name_len;

		//check for end of EIR
		if (field_len == 0)
			return;

		if (offset + field_len > eir_length)
			return;

		if(eir[1] == type) 
		{
			name_len = field_len - 1;
			if (name_len > buf_length)
				return;

			memcpy(buf, &eir[2], name_len);
			return;
		}

		offset += field_len + 1;
		eir += field_len + 1;
	}
}

volatile int ble::signal_received = 0;
