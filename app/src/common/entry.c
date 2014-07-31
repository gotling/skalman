#include <pebble.h>
#include "entry.h"
#include "tools.h"

static GBitmap *action_icon_plus;
static GBitmap *action_icon_confirm;
static GBitmap *action_icon_minus;

static ActionBarLayer *action_bar;

static struct EntryUi {
	Window *window;
	TextLayer *title_text;
	char *title;
	TextLayer *entry_text;
} ui;

static struct EntryState {
	int *entry;
	int step;
	char *format;
	EntryType entry_type;
	TimeType time_type;
	bool callback;
	void (*callback_function)(void);
	char* (*lookup_function)(char *buf, int direction);
	int choices_min;
	int choices_max;
	bool zero_allowed;
	ActionType action_type;
} state;

static char buf[20];

static void update_ui() {
	if (state.entry_type == TIME) {
		format_time_long(buf, *state.entry);
	} else if (state.entry_type == ENUM) {
		state.lookup_function(buf, *state.entry);
	} else {
		snprintf(buf, 20, state.format, *state.entry);
	}

	text_layer_set_text(ui.entry_text, buf);
	layer_mark_dirty(text_layer_get_layer(ui.entry_text));
}

static int get_time_step(bool up) {
	if ((up && *state.entry <= 55) || (!up && *state.entry <= 60)) {
		return 5;
	} else if((up && *state.entry <=170) || (!up && *state.entry <=180)) {
		return 10;
	} else if ((up && *state.entry <= 570) || (!up && *state.entry <= 600)) {
		return 30;
	} else {
		return 60;
	}
}

static int get_number_step(bool up) {
	if ((up && *state.entry <= 9) || (!up && *state.entry <= 10)) {
		return 1;
	} else if ((up && *state.entry <= 40) || (!up && *state.entry <= 50)) {
		return 10;
	} else if((up && *state.entry <=125) || (!up && *state.entry <=150)) {
		return 25;
	} else if ((up && *state.entry <= 250) || (!up && *state.entry <= 300)) {
		return 50;
	} else if ((up && *state.entry <= 900) || (!up && *state.entry <= 1000)) {
		return 100;
	} else if ((up && *state.entry <= 9000) || (!up && *state.entry <= 10000)) {
		return 1000;
	} else {
		return 10000;
	}
}

static int get_step(bool up) {
	if (state.entry_type == TIME && state.time_type != TIME_STEP) {
		return get_time_step(up);
	} else if (state.entry_type == NUMBER_STEP) {
		return get_number_step(up);
	} else {
		return state.step;
	}
}

static void up_click_handler(ClickRecognizerRef recognizer, void* context) {
	if (state.entry_type == ENUM) {
		if (*state.entry < state.choices_max) {
			*state.entry += 1;
		}
	} else {
		*state.entry += get_step(true);
	}
	update_ui();
}

static void down_click_handler(ClickRecognizerRef recognizer, void* context) {
	if (state.entry_type == ENUM) {
		if (*state.entry > state.choices_min) {
			*state.entry -= 1;
		}
	} else if ((*state.entry - get_step(false) > 0) || (state.zero_allowed && (*state.entry - get_step(false) >= 0))) {
		*state.entry -= get_step(false);
	}
	update_ui();
}

static void select_click_handler(ClickRecognizerRef recognizer, void* context) {
	state.action_type = ACTION_CONFIRM;
	window_stack_pop(true);
}

static void click_config_provider(void* context) {
	const uint16_t repeat_interval_ms = 100;
	window_set_click_context(BUTTON_ID_UP, context);
	window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, up_click_handler);
	
	window_set_click_context(BUTTON_ID_SELECT, context);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	
	window_set_click_context(BUTTON_ID_DOWN, context);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, down_click_handler);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	bounds.size.w = bounds.size.w - ACTION_BAR_WIDTH -3;
	
	action_bar = action_bar_layer_create();
	action_bar_layer_add_to_window(action_bar, ui.window);
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

	action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_plus);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_confirm);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_minus);
	
	ui.title_text = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 30 }});
	text_layer_set_text(ui.title_text, ui.title);
	text_layer_set_text_alignment(ui.title_text, GTextAlignmentCenter);
	text_layer_set_font(ui.title_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(window_layer, text_layer_get_layer(ui.title_text));
	
	ui.entry_text = text_layer_create((GRect) { .origin = { 5, bounds.size.h / 2 - 18 }, .size = { bounds.size.w -5, bounds.size.h }});
	text_layer_set_text_alignment(ui.entry_text, GTextAlignmentCenter);
	text_layer_set_font(ui.entry_text, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	//text_layer_set_overflow_mode(ui.entry_text, GTextOverflowModeFill);
	text_layer_set_overflow_mode(ui.entry_text, GTextOverflowModeTrailingEllipsis);
	layer_add_child(window_layer, text_layer_get_layer(ui.entry_text));
	
	update_ui();
}

static void window_unload(Window *window) {
	text_layer_destroy(ui.title_text);
	text_layer_destroy(ui.entry_text);
	
	gbitmap_destroy(action_icon_plus);
	gbitmap_destroy(action_icon_confirm);
	gbitmap_destroy(action_icon_minus);
	
	action_bar_layer_destroy(action_bar);
	
	if(state.callback && state.action_type == ACTION_CONFIRM) {
		state.callback_function();
	}

	window_destroy(window);
}

static void entry_init(char *title, int *entry) {
	ui.title = title;
	state.entry = entry;

	action_icon_plus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLUS);
	action_icon_confirm = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_CONFIRM);
	action_icon_minus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_MINUS);
	
	ui.window = window_create();
	window_set_click_config_provider(ui.window, click_config_provider);
	window_set_window_handlers(ui.window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	const bool animated = true;
	window_stack_push(ui.window, animated);
}

void entry_init_number(char *title, char *format, int step, int *entry) {
	state.entry_type = NUMBER;
	state.step = step;
	state.format = format;
	state.zero_allowed = false;

	entry_init((char*) title, (int*) entry);
}

void entry_init_number_step(char *title, char *format, int *entry, void (*callback)(void)) {
	state.entry_type = NUMBER_STEP;
	state.format = format;
	state.zero_allowed = false;

	if (callback != NULL) {
		state.callback = true;
		state.callback_function = callback;
	} else {
		state.callback = false;
	}

	entry_init((char*) title, (int*) entry);
}

void entry_init_time(char *title, int *entry) {
	state.entry_type = TIME;
	state.time_type = TIME_NORMAL;
	state.zero_allowed = false;

	entry_init((char*) title, (int*) entry);
}

void entry_init_time_zero_allowed(char *title, int *entry) {
	state.entry_type = TIME;
	state.time_type = TIME_NORMAL;
	state.zero_allowed = true;

	entry_init((char*) title, (int*) entry);
}

void entry_init_time_callback(char *title, int *entry, void (*callback)(void)) {
	entry_init_time((char*)title, (int*)entry);

	state.time_type = TIME_NORMAL;
	state.callback = true;
	state.callback_function = callback;
}

void entry_init_time_step(char *title, int step, int *entry) {
	entry_init_time((char*)title, (int*)entry);

	state.time_type = TIME_STEP;
	state.step = step;
}

void entry_init_enum(char *title, char* (*lookup_function)(char *buf, int direction), int choices_max, int *entry) {
	state.entry_type = ENUM;
	state.lookup_function = lookup_function;
	state.choices_min = 1;
	state.choices_max = choices_max;
	state.step = 1;
	state.zero_allowed = false;
	state.action_type = ACTION_CANCEL;

	entry_init((char*) title, (int*) entry);
}

// Preferably this should call on function entry_init_enum but I could
// not get past "error: 'direction' undeclared (first use in this function)"
void entry_init_enum_callback(char *title, char* (*lookup_function)(char *buf, int direction), int choices_min, int choices_max, int *entry, void (*callback)()) {
	state.entry_type = ENUM;
	state.lookup_function = lookup_function;
	state.choices_min = choices_min;
	state.choices_max = choices_max;
	state.step = 1;
	state.zero_allowed = true;

	state.callback = true;
	state.callback_function = callback;

	state.action_type = ACTION_CANCEL;

	entry_init((char*) title, (int*) entry);
}

void entry_deinit(void) {
	window_destroy(ui.window);
}
