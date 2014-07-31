#pragma once

int sustenance_storage_get_sustenance_count();
void sustenance_storage_write_sustenance(struct Sustenance *sustenance);
void sustenance_storage_read_sustenance(struct Sustenance *sustenance, int id);
void sustenance_storage_update_sustenance(int id, struct Sustenance *sustenance);
struct Sustenance *sustenance_storage_read_sustenance_all(struct Sustenance *sustenance);
struct Sustenance *sustenance_storage_read_sustenance_some(struct Sustenance *sustenance, int count);