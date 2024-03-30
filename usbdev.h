#ifndef USBDEV_H_
#define USBDEV_H_

#include <stdio.h>
#include <stdbool.h>
#include <libusb-1.0/libusb.h>

bool fx2IsOpen(libusb_device_handle *usbHandle);



#endif /* End of usbdev.h */
