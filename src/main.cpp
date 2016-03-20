#include "ble.h"

#include <cstdio>

int main(int argc, char **argv)
{
	int device_id = -1;
	ble _ble;
	_ble.list_devices();
	printf("Podaj numer urzadzenia: ");
	scanf("%d", &device_id);
	_ble.scan_le_devices(device_id);
}