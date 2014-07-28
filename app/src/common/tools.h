#pragma once

#define MENU_CELL_BASIC_HEIGHT 44
#define MENU_CELL_BASIC_MULTILINE_HEIGHT 62

char *format_time(char *formated_time, int seconds);
char *format_time_long(char *formated_time, int seconds);
char *format_time_t(char *formated_time, long time_since_epoch);
void menu_cell_basic_draw_multiline(GContext* ctx, const Layer *cell_layer, char *title, char *subtitle, GBitmap *icon);
void menu_cell_basic_draw_multiline_with_extra_title(GContext* ctx, const Layer *cell_layer, char *title, char *right_title, char *subtitle, GBitmap *icon);

static const uint32_t const segments[] = { 200, 100, 400, 100, 600 };
static const VibePattern end_vibration = {
	.durations = segments,
	.num_segments = ARRAY_LENGTH(segments),
};