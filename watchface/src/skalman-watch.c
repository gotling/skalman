#include <pebble.h>
#include "common/time_tools.h"

static struct EntryUi {
	Window *window;
	Layer *layer;
	TextLayer *time_layer;
	TextLayer *date_layer;
} ui;

GColor background_color = GColorBlack;
GColor foreground_color = GColorWhite;

static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00";

  clock_copy_time_string(time_text, sizeof(time_text));
  //strftime(time_text, sizeof(time_text), "%H:%M", tick_time);
  text_layer_set_text(ui.time_layer, time_text);
}

static void handle_day_tick(struct tm* tick_time, TimeUnits units_changed) {
	static char date_text[] = "DD Torsdag";
	static char weekday_text[] = "Torsdag";

	weekday_tostring(weekday_text, tick_time->tm_wday);
	snprintf(date_text, sizeof(date_text), "%d %s", tick_time->tm_mday, weekday_text);

	text_layer_set_text(ui.date_layer, date_text);
}

static void layer_update_callback(Layer *layer, GContext* ctx) {
	GRect bounds = layer_get_frame(layer);
	graphics_context_set_stroke_color(ctx, foreground_color);
 
	int separator_y = bounds.size.h / 2;
	graphics_draw_line(ctx, GPoint(0, separator_y), GPoint(bounds.size.w, separator_y));
	//graphics_fill_circle(ctx, GPoint(bounds.size.w/2, bounds.size.h-110), 180);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "LAYER UPDATE CALLBACK");
}

static void do_init(void) {
	ui.window = window_create();
	window_stack_push(ui.window, true);
	window_set_background_color(ui.window, background_color);

	// Initialize the layer
	Layer *window_layer = window_get_root_layer(ui.window);
	GRect bounds = layer_get_frame(window_layer);
	ui.layer = layer_create(bounds);

	// Set up the update layer callback
	layer_set_update_proc(ui.layer, layer_update_callback);

	// Add the layer to the window for display
	layer_add_child(window_layer, ui.layer);

	ui.time_layer = text_layer_create(GRect(5, 0, 144-5, 49));
	text_layer_set_text_color(ui.time_layer, foreground_color);
	text_layer_set_background_color(ui.time_layer, background_color);
	text_layer_set_text_alignment(ui.time_layer, GTextAlignmentCenter);
	text_layer_set_font(ui.time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));

	ui.date_layer = text_layer_create(GRect(5, 49, 144-5, 28+4));
	text_layer_set_text_color(ui.date_layer, foreground_color);
	text_layer_set_background_color(ui.date_layer, background_color);
	text_layer_set_text_alignment(ui.date_layer, GTextAlignmentCenter);
	text_layer_set_font(ui.date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

	time_t now = time(NULL);
	struct tm *current_time = localtime(&now);
	
	handle_minute_tick(current_time, MINUTE_UNIT);
	tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
	handle_day_tick(current_time, DAY_UNIT);
	tick_timer_service_subscribe(DAY_UNIT, &handle_day_tick);

	layer_add_child(window_layer, text_layer_get_layer(ui.time_layer));
	layer_add_child(window_layer, text_layer_get_layer(ui.date_layer));
}

static void do_deinit(void) {
	text_layer_destroy(ui.date_layer);
	text_layer_destroy(ui.time_layer);
	window_destroy(ui.window);
}

int main(void) {
	do_init();
	app_event_loop();
	do_deinit();
}