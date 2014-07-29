#include <pebble.h>
#include "sustenance/sustenance_data.h"

#define SUSTENANCE_COUNT_PKEY 10

#define SUSTENANCE_COUNT_DEFAULT 0

const int sustenance_first_pkey = 100;
int sustenance_count = SUSTENANCE_COUNT_DEFAULT;


int storage_get_sustenance_count() {
	return persist_exists(SUSTENANCE_COUNT_PKEY) ? persist_read_int(SUSTENANCE_COUNT_PKEY) : SUSTENANCE_COUNT_DEFAULT;
}

void storage_update_sustenance_count() {
	int count = storage_get_sustenance_count() + 1;
	int valueWritten = persist_write_int(SUSTENANCE_COUNT_PKEY, count);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "STORAGE. WRITE SUSTENANCE. PKEY: %d. ID: %d. Bytes written: %d", SUSTENANCE_COUNT_PKEY, count, valueWritten);
}

void storage_write_sustenance(struct Sustenance *sustenance) {
	int next_id = (sustenance_first_pkey + storage_get_sustenance_count());
	//int next_id = 100;
	int bytesWritten = persist_write_data(next_id, sustenance, sizeof(struct Sustenance));
	int expectedBytesWritten = sizeof(struct Sustenance);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "STORAGE. WRITE SUSTENANCE. PKEY: %d. Expected bytes written: %d. Bytes written: %d", next_id, expectedBytesWritten, bytesWritten);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "STORAGE. WRITE SUSTENANCE. Type: %d. Substance: %d. Amount: %d", sustenance->type, sustenance->substance, sustenance->amount);
	storage_update_sustenance_count();
}

void storage_read_sustenance(struct Sustenance *sustenance, int id) {
	int bytesRead = persist_read_data(id, sustenance, sizeof(struct Sustenance));
	APP_LOG(APP_LOG_LEVEL_DEBUG, "STORAGE. READ SUSTENANCE. ID: %d. Bytes read: %d", id, bytesRead);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "STORAGE. READ SUSTENANCE. Type: %d. Substance: %d. Amount: %d", sustenance->type, sustenance->substance, sustenance->amount);
}

struct Sustenance *storage_read_sustenance_all(struct Sustenance *sustenance) {
	int count = storage_get_sustenance_count();
	for (int i = 0; i < count; i++) {
		storage_read_sustenance(&sustenance[count -1 -i], sustenance_first_pkey + i);
	}

	return sustenance;
}