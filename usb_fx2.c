#include "usb_fx2.h"


/*
 * @brief check if fx2 is open
 */
static inline bool fx2_is_open(libusb_device_handle *devHandle) {
	return(devHandle);
}

/*
 * write a chunk of data to the fx2 ram
 */
int fx2_write_ram(libusb_device_handle *devHandle, size_t address, const unsigned char *data, size_t nBytes) {
	// Check if device is connected
	if(!fx2_is_open(devHandle)) {
		fprintf(stderr, "fx2_write_ram: Not connected!\n");
		return(1);
	}

	// Num of error tries
	int nErrors = 0;

	const size_t chunkSize = 16;	// only 16 bytes are allower per transfer
	const unsigned char *d = data;
	const unsigned char *dEnd = data + nBytes;
	while(d < dEnd) {
		size_t byteSize = dEnd - d;
		if(byteSize > chunkSize) byteSize = chunkSize;
		size_t dlAddr = address + (d - data);
		int rv = libusb_control_transfer(devHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
										 FX2LP_CMD_REQUEST, dlAddr/*wValue*/, 0/*wIndex*/, (unsigned char*)d/*data*/, byteSize/*wLength*/, FX2LP_CONTROL_TIMEOUT/*timeout*/);
		if(rv < 0) {
			fprintf(stderr, "Writing %zu bytes at 0x%zx: %s\n", byteSize, dlAddr, libusb_strerror(rv));
			++nErrors;
		}
		d += byteSize;
	}
	return(nErrors);
}

/*
 * @brief read a chunk of data from fx2
 */
int fx2_read_ram(libusb_device_handle *devHandle, size_t address, unsigned char *data, size_t nBytes) {
	// Check if fx2 is available and opened
	if(!fx2_is_open(devHandle)) {
		fprintf(stderr, "fx2_read_ram: Not connected!\n");
		return(1);
	}

	int nErrors = 0;

	const size_t chunkSize = 16;	// Only 16 bytes maximum allowed per transfer

	unsigned char *d = data;
	unsigned char *dEnd = data + nBytes;

	while(d < dEnd) {
		size_t byteSize = dEnd - d;
		if(byteSize > chunkSize) byteSize = chunkSize;
		size_t rdAddr = address + (d - data);
		int rv = libusb_control_transfer(devHandle, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
										 FX2LP_CMD_REQUEST/*bRequest*/, rdAddr/*wValue*/, 0/*wIndex*/, data/*data*/, nBytes/*wLength*/, FX2LP_CONTROL_TIMEOUT/*timeout*/);
		if(rv < 0) {
			fprintf(stderr, "Reading %zu bytes at 0x%zx: %s\n", byteSize, rdAddr, libusb_strerror(rv));
			++nErrors;
		}
		d += byteSize;
	}
	return(nErrors);
}

/*
 * @brief send reset signal to the fx2
 */
bool fx2_send_reset(libusb_device_handle *devHandle, bool reset) {
	int rv = 0;	// libusb return status
	uint8_t cpucs = reset;	// Reset state

	rv = fx2_write_ram(devHandle, FX2LP_CPUCS_ADDR, &cpucs, 1);
	return(rv == 0 ? true : false);
}