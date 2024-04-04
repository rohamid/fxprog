#ifndef USB_BENCHMARK_H_
#define USB_BENCHMARK_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <libusb-1.0/libusb.h>

typedef struct {
	unsigned char endpoint;
	uint8_t command;
	int transferSize;
}usb_device_config_t;

int usb_bulk_write_ep(libusb_device_handle *devHandle, usb_device_config_t *udev, uint8_t *data, size_t data_len);
int usb_bulk_read_ep(libusb_device_handle *devHandle, usb_device_config_t *udev, uint8_t *data, size_t data_len);
int usb_speed_test(libusb_device_handle *devHandle, usb_device_config_t *udev_in, usb_device_config_t *udev_out, uint8_t *data_in, uint8_t *data_out, int data_len, int test_num);

#endif /* End of usb_benchmark */
