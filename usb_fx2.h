#ifndef USB_FX2_H_
#define USB_FX2_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include "ihex.h"


#define FX2LP_CMD_REQUEST		0xA0
#define FX2LP_CPUCS_ADDR		0xE600
#define FX2LP_CONTROL_TIMEOUT	1000

int fx2_write_ram(libusb_device_handle *devHandle, size_t address, const unsigned char *data, size_t nBytes);
int fx2_read_ram(libusb_device_handle *devHandle, size_t address, unsigned char *data, size_t nBytes);
bool fx2_send_reset(libusb_device_handle *devHandle, bool reset);
int fx2_write_ihex_line(libusb_device_handle *devHandle, const char *lineRecord);
int fx2_write_ihex(libusb_device_handle *devHandle, const char *filePath);


#endif /* End of usb_fx2 */
