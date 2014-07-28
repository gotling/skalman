#pragma once

int storage_get_sustenance_count();
void storage_write_sustenance(struct Sustenance *sustenance);
void storage_read_sustenance(struct Sustenance *sustenance, int id);
struct Sustenance *storage_read_sustenance_all(struct Sustenance *sustenance);
struct Sustenance *storage_read_sustenance_and_malloc(int id);