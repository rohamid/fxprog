#ifndef USBDEV_H_
#define USBDEV_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

void usb_error_check(int error_check, const char *str);
bool fx2IsOpen(libusb_device_handle *usbHandle);
void usb_init(libusb_context **ctx);
bool usb_open(libusb_device_handle **devHandle, libusb_context **ctx, int vid, int pid);
void usb_close(libusb_device_handle *devHandle, libusb_context *ctx);

#endif /* End of usbdev.h */
