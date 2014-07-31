#pragma once

void sustenance_report_init();
void sustenance_report_init_new();
void sustenance_report_init_edit(int id, void (*callback_function)(void));
void sustenance_report_deinit();