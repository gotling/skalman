#include <pebble.h>
#include "common/tools.h"
#include "sustenance/sustenance_report.h"
#include "sustenance/sustenance_history.h"
#include "sustenance/sustenance_data.h"
#include "sustenance/sustenance_storage.h"

#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 2

static Window *window;
static TextLayer *header;
static MenuLayer *menu_layer;

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
	return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	switch (section_index) {
		case 0:
			return NUM_FIRST_MENU_ITEMS;
		default:
			return 0;
	}
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	return 0;//MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
	// switch (section_index) {
	// 	case 0:
	// 		menu_cell_basic_header_draw(ctx, cell_layer, "Digestion");
	// 		break;
	// }
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	switch (cell_index->section) {
		default:
			return MENU_CELL_BASIC_HEIGHT;
	}
}

static char subbuf[20];
static int sustenance_count;

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	switch (cell_index->section) {
		case 0:
			switch (cell_index->row) {
				case 0:
					menu_cell_basic_draw(ctx, cell_layer, "Report", "Source of strength", NULL);
					//menu_cell_basic_draw_multiline_with_extra_title(ctx, cell_layer, "sustenance", NULL, NULL, NULL);
					break;
				case 1:
					sustenance_count = storage_get_sustenance_count();
					snprintf(subbuf, 20, "Done %d so far", sustenance_count);
					menu_cell_basic_draw(ctx, cell_layer, "History", subbuf, NULL);
					break;
				default:
					break;
			}
			break;
	}
}

void reload_menu(void) {
	menu_layer_reload_data(menu_layer);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	switch (cell_index->section) {
		case 0:
			switch (cell_index->row) {
				case 0:
					sustenance_report_init_new();
					break;
				case 1:
					sustenance_history_init();
					break;
			}
			break;
	}
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	header = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 32 } });
	text_layer_set_text(header, "Skalman");
	text_layer_set_text_alignment(header, GTextAlignmentCenter);
	text_layer_set_font(header, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	layer_add_child(window_layer, text_layer_get_layer(header));
	
	menu_layer = menu_layer_create((GRect) { .origin = { 0, 40 }, .size = { bounds.size.w, 114 } });
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.get_cell_height = menu_get_cell_height_callback,
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
}

void menu_init(void) {
	// CLEAR
	//persist_write_int(10, 0);

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	
	const bool animated = true;
	window_stack_push(window, animated);
}

void menu_deinit(void) {
	window_destroy(window);
}