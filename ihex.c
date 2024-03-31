#include "ihex.h"

#define HI(x)	(((x) & 0xff00) >> 8)
#define LO(x)	((x) & 0xff)

static inline unsigned char ihex_get_checksum(const char *record) {
	int recordLen = strlen(record);
	unsigned char checksum = 0;

	// Skip the colon at the beginning of the record
	// Also skip the last 3 bytes. It contain the checksum itself and null terminated.
	for(int i = 1; i < recordLen - 3; i += 2) {
		// Convert two characters representing a byte into an actual byte
		char byte_str[3] = {record[i], record[i+1], '\0'};
		unsigned char byte = (unsigned char)strtol(byte_str, NULL, 16);

		// Add the byte to the checksum
		checksum += byte;
	}

	// Take the one's complement
	checksum = ~checksum;

	// Add 1 to obtain the two's complement
	checksum += 1;

	return checksum;

	// Or just do this instead
	//return (~checksum) + 1;
}

/*
 * @brief calculate data size of given ihex file
 */
int ihex_get_data_size(const char *filePath, unsigned int *dataSize) {
	unsigned int chunkSize, line=0;
	static char lineBuff[523];

	// Open ihex file
	FILE *ihex = fopen(filePath, "r");
	if(ihex == NULL) {
		fprintf(stderr, "Error opening IHEX file\n");
		return(1);
	}

	// Read ihex file line by line
	while(fgets(lineBuff, sizeof(lineBuff), ihex)) {
		// Ignore char but ':', as ':' is the first character 
		// ihex specifier
		if(sscanf(lineBuff, ":%02x", &chunkSize) != 1) {
			fprintf(stderr, "Error while parsing IHEX at line %d\n", line);
			if(ihex) {
				rewind(ihex);
				fclose(ihex);
				return(1);
			}
		}
		// Accumulate data size
		*dataSize += chunkSize;
		line++;
	}
	// Close and put the file pointer to the 1st location
	if(ihex) {
		rewind(ihex);
		fclose(ihex);
	}
	return(0);
}

int ihex_parse_record(char *line, hex_record_t *record) {
	int byteCount, recordType;
	unsigned int address;
	int i;

	// Check if the line starts with ':'
	if(line[0] != ':') {
		printf("ihex_parse_record: Not a valid intel hex record!\n");
		return(-1);
	}

	// Parse record fields
	if(sscanf(line, ":%2x%4x%2x", &byteCount, &address, &recordType) != 3) {
		printf("ihex_parse_record: Error parsing hex file!\n");
		return(-1);
	}

	// Allocate memory for data
	record->data = (unsigned char*)malloc(byteCount * sizeof(unsigned char));
	if(record->data == NULL) {
		return(-2);	// Memory allocation failed
	}

	// Populate record fields
	record->address = address;
	record->length = byteCount;
	record->type = recordType;
	
	
	// Parse data bytes
	for(i = 0; i < byteCount; i++) {
		sscanf(line + 9 + i * 2, "%2x", &record->data[i]);
	}

	if(sscanf(line+9+byteCount*2, "%2x", &record->checkSum) != 1) {
		printf("Failed to get line checksum!\n");
		return(-1);
	}

	return(0); 	// Parsing succesfull
}

void ihex_dump_file(const char *fileName) {
	FILE *pFile;
	char line[MAX_RECORD_SIZE];
	hex_record_t record;
	int lineNumber = 0;

	// Open file for reading
	pFile = fopen(fileName, "r");
	if(pFile == NULL) {
		printf("ihex_dump_file: Error opening file!\n");
		return;
	}

	// Read and parse each line of the file
	while(fgets(line, MAX_RECORD_SIZE, pFile) != NULL) {
		lineNumber++;
		if(ihex_parse_record(line, &record) == 0) {
			// Dump record contents
			printf("ihex_dump_file: Line %d - Address: 0x%04x, Length: %d bytes, Data: ", lineNumber, record.address, record.length);
			for(int i = 0; i < record.length; i++) {
				printf("%02x ", record.data[i]);
			}
			//printf("SUM: %2x ", record.checkSum);
			printf("SUM: %2x ", ihex_get_checksum(line));
			printf("\n");

			// Free memory allocated for data
			free(record.data);
		} else {
			printf("ihex_dump_file: Error parsing line %d: Not a valid Intel HEX record\n", lineNumber);
		}
	}
	fclose(pFile);
}
