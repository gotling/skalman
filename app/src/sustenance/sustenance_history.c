#include <pebble.h>
#include "sustenance_data.h"
#include "sustenance_report.h"
#include "sustenance_storage.h"
#include "../common/tools.h"

#define NUM_MENU_SECTIONS 2
#define NUM_SECOND_MENU_ITEMS 1

static Window *window;
static TextLayer *header;
static MenuLayer *menu_layer;

static int sustenance_count = 0;
static struct Sustenance *sustenances;
static struct Sustenance sustenance;

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
	return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	switch (section_index) {
		case 0:
			return sustenance_count;
		case 1:
			return NUM_SECOND_MENU_ITEMS;
		default:
			return 0;
	}
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
	switch (section_index) {
		case 0:
			menu_cell_basic_header_draw(ctx, cell_layer, "History");
			break;
		case 1:
			menu_cell_basic_header_draw(ctx, cell_layer, "Operation");
			break;
	}
}

static char title_text[20];
static char time_text[8];
static char sub_text[20];
static int sustenance_unit;

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	switch (cell_index->section) {
		case 0:
			sustenance = sustenances[sustenance_count -1 -cell_index->row];
			sustenance_unit = sustenance_get_unit_for_substance((sustenance).substance);
			sustenance_amount_to_string(sub_text, (sustenance).amount, sustenance_unit);
			sustenance_substance_to_string(title_text, (sustenance).substance);
			format_time_t(time_text, (sustenance).time);
			GBitmap *menu_image = sustenance_image_by_type((sustenance).type);
			
			menu_cell_basic_draw_multiline_with_extra_title(ctx, cell_layer, title_text, time_text, sub_text, menu_image);
			break;
		case 1:
			switch (cell_index->row) {
				case 0:
					menu_cell_basic_draw(ctx, cell_layer, "Clear", NULL, NULL);
					break;
			}
			break;
	}
}

static void reload_menu(void) {
	sustenance_storage_read_sustenance_all(sustenances);
	menu_layer_reload_data(menu_layer);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	switch (cell_index->section) {
		case 0:
			APP_LOG(APP_LOG_LEVEL_DEBUG, "HISTORY. CALLBACK. EDIT");
			sustenance_report_init_edit(sustenance_count -1 -cell_index->row, &reload_menu);
			break;
		case 1:
			switch (cell_index->row) {
				case 0:
					persist_write_int(10, 0);
					APP_LOG(APP_LOG_LEVEL_DEBUG, "HISTORY. CALLBACK. CLEAR");
					window_stack_pop(true);
					break;
			}
			break;

	}
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	
	sustenance_data_init();
	
	menu_layer = menu_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, bounds.size.h } });
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
	});
	menu_layer_set_click_config_onto_window(menu_layer, window);
	layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy(header);
	menu_layer_destroy(menu_layer);

	sustenance_data_deinit();

	window_destroy(window);
}

static void restore_history() {
	sustenance_count = sustenance_storage_get_sustenance_count();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "HISTORY. INIT. SUSTENANCE COUNT: %d", sustenance_count);
	sustenances = malloc(sustenance_count * sizeof(struct Sustenance));
	sustenance_storage_read_sustenance_all(sustenances);
}

void sustenance_history_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	const bool animated = true;

	restore_history();

	window_stack_push(window, animated);
}

void sustenance_history_deinit(void) {
	window_destroy(window);
}