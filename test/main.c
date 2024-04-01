#include <stdio.h>

typedef struct {
    unsigned int address;
    int length;
    int type;
    unsigned char *data;
    unsigned int checkSum;
} hex_record_t;

void dump_hex_record(const hex_record_t *record) {
    printf("[%04X] ", record->address);

    for (int i = 0; i < record->length; i += 4) {
        for (int j = 0; j < 4 && i + j * 2 < record->length; j++) {
            printf("%02X%02X ", record->data[i + j * 2], record->data[i + j * 2 + 1]);
        }

        // Print newline after every 8 bytes
        printf("\n");
        if (i + 4 < record->length)
            printf("[%04X] ", record->address + (i + 4));
    }
}

int main() {
    // Example hex record
    hex_record_t record = {
        .address = 0x0200,
        .length = 16,
        .type = 0,
        .data = (unsigned char[]) {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F,
                                   0x72, 0x6C, 0x64, 0x21, 0x20, 0x54, 0x68, 0x69},
        .checkSum = 0x1234
    };

    // Dump hex record
    dump_hex_record(&record);

    return 0;
}

