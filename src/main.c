#include <pebble.h>
#include "watchface.h"

static Window *s_main_window;
static TextLayer *s_time_layer;

static void update_time() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    
    static char buffer[] = "00:00";
    if(clock_is_24h_style() == true) {
        strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
    } else {
        strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
    }
    
    if(buffer[0] == '0') {
        for(unsigned i = 0; i < sizeof(buffer) - 1; i++) {
            buffer[i] = buffer[i+1];
        }
    }
    
    text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
    if(units_changed & HOUR_UNIT) {
        vibes_short_pulse();
    }
}

static void main_window_load(Window *window) {
    s_time_layer = text_layer_create(GRect(0,58,144,52));
    text_layer_set_background_color(s_time_layer, GColorBlack);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
    update_time();
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
}

static void init() {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    
    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load, 
        .unload = main_window_unload
    });
    window_set_background_color(s_main_window, GColorBlack);
    
    window_stack_push(s_main_window, true);
}

static void deinit() {
    window_destroy(s_main_window);
}

int main(void) {
  //init();
    show_watchface();
    app_event_loop();
    hide_watchface();
  //deinit();
}