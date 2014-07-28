#include <pebble.h>
#include "sustenance_data.h"
#include "../common/entry.h"
#include "../common/tools.h"
#include "../storage.h"

#define NUM_MENU_SECTIONS 2
#define NUM_FIRST_MENU_ITEMS 3
#define NUM_SECOND_MENU_ITEMS 1

static Window *window;
static TextLayer *header;
static MenuLayer *menu_layer;

static void type_entry();
static void substance_entry();
static void amount_entry();

typedef enum {
	NEW,
	EDIT
} Mode;

static struct SustenanceState {
	Mode mode;
	int unit;
	int substance_min;
	int substance_max;
} state;

static struct Sustenance sustenance;

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
	return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	switch (section_index) {
		case 0:
			return NUM_FIRST_MENU_ITEMS;
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
			menu_cell_basic_header_draw(ctx, cell_layer, "Sustenance");
			break;
		case 1:
			menu_cell_basic_header_draw(ctx, cell_layer, "Operation");
			break;
	}
}

static char subbuf[20];

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	switch (cell_index->section) {
		case 0:
			switch (cell_index->row) {
				case 0:
					sustenance_type_to_string(subbuf, sustenance.type);
					menu_cell_basic_draw(ctx, cell_layer, "Type", subbuf, NULL);
					break;
				case 1:
					sustenance_substance_to_string(subbuf, sustenance.substance);
					menu_cell_basic_draw(ctx, cell_layer, "Substance", subbuf, NULL);
					break;
				case 2:
					sustenance_amount_to_string(subbuf, sustenance.amount, state.unit);
					menu_cell_basic_draw(ctx, cell_layer, "Amount", subbuf, NULL);
					break;
			}
			break;
		case 1:
			switch (cell_index->row) {
				case 0:
					menu_cell_basic_draw(ctx, cell_layer, "Save", NULL, NULL);
			}
			break;
	}
}

static void set_substance() {
	state.substance_min = sustenance_get_substance_min(sustenance.type);
	state.substance_max = sustenance_get_substance_max(sustenance.type);
	sustenance.substance = state.substance_min;
}

static void type_updated(void) {
	set_substance();
	menu_layer_set_selected_next(menu_layer, false, MenuRowAlignCenter, false);
	substance_entry();
}

static void set_amount() {
	state.unit = sustenance_get_unit_for_substance(sustenance.substance);
	sustenance.amount = sustenance_get_amount_default(state.unit);
}

static void substance_updated() {
	set_amount();
	menu_layer_set_selected_next(menu_layer, false, MenuRowAlignCenter, false);
	amount_entry();
}

static void type_entry() {
	entry_init_enum_callback("Type", &sustenance_type_to_string, 1000, 1002, (int*)&sustenance.type, &type_updated);
}

static void substance_entry() {
	entry_init_enum_callback("Substance", &sustenance_substance_to_string, state.substance_min, state.substance_max, (int*)&sustenance.substance, &substance_updated);
}

static void amount_updated() {
	menu_layer_set_selected_next(menu_layer, false, MenuRowAlignCenter, false);
}

static char formbuf[20];

static void amount_entry() {
	strcpy(formbuf, "%d ");
	char unit_text[12];
	sustenance_unit_to_string(unit_text, state.unit);
	strncat(formbuf, unit_text, sizeof unit_text);
	entry_init_number_step("Amount", formbuf, (int*)&sustenance.amount, &amount_updated);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	switch (cell_index->section) {
		case 0:
			switch (cell_index->row) {
				case 0:
					type_entry();
					break;
				case 1:
					substance_entry();
					break;
				case 2:
					amount_entry();
					break;
			}
			break;
		case 1:
			switch (cell_index->row) {
				case 0:
					sustenance.time = time(NULL);
					storage_write_sustenance(&sustenance);
					window_stack_pop(true);
					break;
			}
			break;

	}
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	
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
	
	window_destroy(window);
}

void sustenance_report_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	const bool animated = true;

	sustenance = sustenance_get_default();
	set_substance();
	set_amount();
	state.unit = MILLIGRAM;

	window_stack_push(window, animated);
}

void sustenance_report_init_new(void) {
	state.mode = NEW;
	sustenance_report_init();
	type_entry();
}

void sustenance_report_init_edit(int id) {
	state.mode = EDIT;
	storage_read_sustenance(&sustenance, id);
	sustenance_report_init();
}

void sustenance_report_deinit(void) {
	window_destroy(window);
}