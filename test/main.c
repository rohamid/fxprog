#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to calculate the checksum for an Intel HEX record
unsigned char calculate_record_checksum(const char *record) {
    int record_length = strlen(record);
    unsigned char checksum = 0;

    // Skip the colon at the beginning of the record
    for (int i = 1; i < record_length - 3; i += 2) {
        // Convert two characters representing a byte into an actual byte
        char byte_str[3] = {record[i], record[i + 1], '\0'};
        unsigned char byte = (unsigned char)strtol(byte_str, NULL, 16);

        // Add the byte to the checksum
        checksum += byte;
    }

    // Take the one's complement
    checksum = ~checksum;

    // Add 1 to obtain the two's complement
    checksum += 1;

    return checksum;
}

int main() {
    // Example Intel HEX record
    char ihex_record[] = ":020000040000FA\n";

    // Calculate checksum
    unsigned char checksum = calculate_record_checksum(ihex_record);

    // Print checksum
    printf("Checksum: 0x%02X\n", checksum);

    return 0;
}
