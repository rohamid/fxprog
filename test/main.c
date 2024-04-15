#include <stdio.h>
#include <libusb-1.0/libusb.h>

void print_device_descriptor(struct libusb_device *dev) {
    struct libusb_device_descriptor desc;
    struct libusb_config_descriptor *config;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
        fprintf(stderr, "Failed to get device descriptor\n");
        return;
    }

    printf("Number of possible configurations: %d\n", desc.bNumConfigurations);
    printf("Device Class: %d\n", desc.bDeviceClass);
    printf("VendorID: %04x\n", desc.idVendor);
    printf("ProductID: %04x\n", desc.idProduct);

    r = libusb_get_config_descriptor(dev, 0, &config);
    if (r < 0) {
        fprintf(stderr, "Failed to get config descriptor\n");
        return;
    }

    printf("Interfaces: %d\n", config->bNumInterfaces);
    libusb_free_config_descriptor(config);

    printf("USB Speed: ");
    switch(libusb_get_device_speed(dev)) {
        case LIBUSB_SPEED_LOW: printf("Low (1.5 Mbit/s)\n"); break;
        case LIBUSB_SPEED_FULL: printf("Full (12 Mbit/s)\n"); break;
        case LIBUSB_SPEED_HIGH: printf("High (480 Mbit/s)\n"); break;
        case LIBUSB_SPEED_SUPER: printf("Super (5 Gbit/s)\n"); break;
        default: printf("Unknown\n");
    }

    printf("USB Address: %d\n", libusb_get_device_address(dev));
    printf("USB Bus number: %d\n", libusb_get_bus_number(dev));
    printf("\n");
}

int main() {
    libusb_device **devs;
    libusb_context *ctx = NULL;
    int r;
    ssize_t cnt;

    r = libusb_init(&ctx);
    if (r < 0) {
        fprintf(stderr, "Init Error %d\n", r);
        return 1;
    }

    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
    cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        fprintf(stderr, "Get Device Error\n");
    }

    printf("Devices found: %ld\n", cnt);
    printf("---------------------------------------\n");

    for (ssize_t i = 0; i < cnt; i++) {
        print_device_descriptor(devs[i]);
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);

    return 0;
}
