#include "usb_benchmark.h"


int usb_bulk_write_ep(libusb_device_handle *devHandle, usb_device_config_t *udev, uint8_t *data, size_t data_len) {
	int rv = 0;
	int transferred = 0;

	rv = libusb_bulk_transfer(devHandle, udev->endpoint, data, udev->transferSize, &transferred, 1000);
	if(0 == rv) {
		if(udev->transferSize == transferred) {
			// Transfer success
			return 0;
		} else {
			printf("usb_bulk_write_ep() error transferred=%u bytes instead %u bytes to endpoint %d\n", transferred, udev->transferSize, udev->endpoint);
			return -1;
		}
	} else {
		printf("usb_bulk_write_ep() return error\n");
		return -2;
	}
}

static float time_diff(const struct timeval *a, const struct timeval *b) {
	return (a->tv_sec - b->tv_sec) + 1e-6f * (a->tv_usec - b->tv_usec);
}

int usb_bulk_read_ep(libusb_device_handle *devHandle, usb_device_config_t *udev, uint8_t *data, size_t data_len) {
	int rv = 0;
	int received = 0;

	rv = libusb_bulk_transfer(devHandle, udev->endpoint, data, udev->transferSize, &received, 1000);
	if(0 == rv) {
		if(udev->transferSize == received) {
			return 0;
		} else {
			printf("usb_bulk_read_ep() error received=%u bytes instead %u bytes from endpoint %d\n", received, udev->transferSize, udev->endpoint);
			return -1;
		}
	} else {
		printf("usb_bulk_read_ep() return error\n");
		return -2;
	}
}

int usb_speed_test(libusb_device_handle *devHandle, usb_device_config_t *udev_in, usb_device_config_t *udev_out, uint8_t *data_in, uint8_t *data_out, int data_len, int test_num) {
	double speed_mbytes_per_sec;
	int testCount;
	size_t totalData;
	
	struct timeval start_time;
	struct timeval end_time;

	float time_diff_s;

	printf("Start usb_speed_test() #1...\n");
	printf("libusb_bulk_transfer(): writeEP=0x%02x | libusb_bulk_transfer() readEP=0x%02x | %d times | %d bytes \n", udev_out->endpoint, udev_in->endpoint, test_num, data_len);

	printf("Start Test1 usb_bulk_write_ep()\n");
	gettimeofday(&start_time, NULL);
	for(testCount = 0; testCount < test_num; ++testCount) {
		if(usb_bulk_write_ep(devHandle, udev_out, data_out, data_len) == 0) {
			totalData += data_len;
		} else {
			printf("usb_bulk_write_ep() error\n");
			printf("Test failure!\n");
			return -1;
		}
	}
	gettimeofday(&end_time, NULL);
	printf("End test\n");

	time_diff_s = time_diff(&end_time, &start_time);
	speed_mbytes_per_sec = (float)(((double)totalData) / (1000.0 * 1000.0)) / time_diff_s;
	printf("usb_bulk_write_ep() average speed %.1f MBytes/Sec, Total=%zu Bytes/%zu MBytes\n", speed_mbytes_per_sec, totalData, (totalData/(1000 * 1000)));

	printf("Start Test1 usb_bulk_read_ep()\n");
	totalData = 0;
	gettimeofday(&start_time, NULL);
	for(testCount = 0; testCount < test_num; ++testCount) {
		if(usb_bulk_read_ep(devHandle, udev_in, data_in, data_len) == 0) {
			totalData += data_len;
		} else {
			printf("usb_bulk_read_ep() error\n");
			printf("Test failure!\n");
			return -1;
		}
	}
	gettimeofday(&end_time, NULL);
	printf("End test\n");

	time_diff_s = time_diff(&end_time, &start_time);
	speed_mbytes_per_sec = (float)(((double)totalData) / (1000.0 * 1000.0)) / time_diff_s;
	printf("usb_bulk_read_ep() average speed %.1f MBytes/Sec, Total=%zu Bytes/%zu MBytes\n", speed_mbytes_per_sec, totalData, (totalData/(1000 * 1000)));

	printf("End usb_speed_test()\n");
	printf("Test end with success\n");
	return 0;
}


