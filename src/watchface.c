#include "watchface.h"
#include <pebble.h>

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_futura_condensed_48;
static GFont s_res_gothic_28_bold;
static GBitmap *s_res_battery_0;
static GBitmap *s_res_bluetooth;
static GBitmap *s_res_empty;
static Layer *sCalendarLayer;
static TextLayer *sTimeLayer;
static TextLayer *sDateLayer;
static TextLayer *sDayLayer;
static BitmapLayer *sBatteryLayer;
static BitmapLayer *sBluetoothLayer;
static BitmapLayer *sChargingLayer;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_fullscreen(s_window, true);
  
  s_res_futura_condensed_48 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_CONDENSED_48));
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  s_res_battery_0 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_0);
  s_res_bluetooth = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH);
  s_res_empty = gbitmap_create_with_resource(RESOURCE_ID_EMPTY);
  // sCalendarLayer
  sCalendarLayer = layer_create(GRect(0, 96, 144, 72));
  layer_add_child(window_get_root_layer(s_window), (Layer *)sCalendarLayer);
  
  // sTimeLayer
  sTimeLayer = text_layer_create(GRect(34, 48, 110, 48));
  text_layer_set_background_color(sTimeLayer, GColorBlack);
  text_layer_set_text_color(sTimeLayer, GColorWhite);
  text_layer_set_text(sTimeLayer, "10:00");
  text_layer_set_text_alignment(sTimeLayer, GTextAlignmentRight);
  text_layer_set_font(sTimeLayer, s_res_futura_condensed_48);
  layer_add_child(window_get_root_layer(s_window), (Layer *)sTimeLayer);
  
  // sDateLayer
  sDateLayer = text_layer_create(GRect(62, -2, 81, 30));
  text_layer_set_background_color(sDateLayer, GColorClear);
  text_layer_set_text_color(sDateLayer, GColorWhite);
  text_layer_set_text(sDateLayer, "00/00");
  text_layer_set_text_alignment(sDateLayer, GTextAlignmentRight);
  text_layer_set_font(sDateLayer, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)sDateLayer);
  
  // sDayLayer
  sDayLayer = text_layer_create(GRect(34, 22, 108, 32));
  text_layer_set_background_color(sDayLayer, GColorClear);
  text_layer_set_text_color(sDayLayer, GColorWhite);
  text_layer_set_text(sDayLayer, "Wednesday");
  text_layer_set_text_alignment(sDayLayer, GTextAlignmentRight);
  text_layer_set_font(sDayLayer, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)sDayLayer);
  
  // sBatteryLayer
  sBatteryLayer = bitmap_layer_create(GRect(2, 2, 35, 19));
  bitmap_layer_set_bitmap(sBatteryLayer, s_res_battery_0);
  layer_add_child(window_get_root_layer(s_window), (Layer *)sBatteryLayer);
  
  // sBluetoothLayer
  sBluetoothLayer = bitmap_layer_create(GRect(39, 2, 11, 19));
  bitmap_layer_set_bitmap(sBluetoothLayer, s_res_bluetooth);
  layer_add_child(window_get_root_layer(s_window), (Layer *)sBluetoothLayer);
  
  // sChargingLayer
  sChargingLayer = bitmap_layer_create(GRect(52, 2, 9, 19));
  bitmap_layer_set_bitmap(sChargingLayer, s_res_empty);
  layer_add_child(window_get_root_layer(s_window), (Layer *)sChargingLayer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  layer_destroy(sCalendarLayer);
  text_layer_destroy(sTimeLayer);
  text_layer_destroy(sDateLayer);
  text_layer_destroy(sDayLayer);
  bitmap_layer_destroy(sBatteryLayer);
  bitmap_layer_destroy(sBluetoothLayer);
  bitmap_layer_destroy(sChargingLayer);
  fonts_unload_custom_font(s_res_futura_condensed_48);
  gbitmap_destroy(s_res_battery_0);
  gbitmap_destroy(s_res_bluetooth);
  gbitmap_destroy(s_res_empty);
}
// END AUTO-GENERATED UI CODE

#define DEVICE_WIDTH        144
#define DEVICE_HEIGHT       168


#define CAL_DAYS   7   // number of columns (days of the week)
#define CAL_WIDTH  20  // width of columns
#define CAL_GAP    1   // gap around calendar
#define CAL_LEFT   2   // left side of calendar
#define CAL_HEIGHT 18  // How tall rows should be depends on how many weeks there are

struct tm *currentTime;

static GBitmap *s_res_charging;
static GBitmap *s_res_battery_20;
static GBitmap *s_res_battery_40;
static GBitmap *s_res_battery_60;
static GBitmap *s_res_battery_80;
static GBitmap *s_res_battery_100;

static AppTimer *sTimer;

static bool btStatus;

int daysInMonth(int mon, int year) {
    mon++; // dec = 0|12, lazily optimized

    // April, June, September and November have 30 Days
    if (mon == 4 || mon == 6 || mon == 9 || mon == 11) {
        return 30;
    } else if (mon == 2) {
        // Deal with Feburary & Leap years
        if (year % 400 == 0) {
            return 29;
        } else if (year % 100 == 0) {
            return 28;
        } else if (year % 4 == 0) {
            return 29;
        } else {
            return 28;
        }
    } else {
        // Most months have 31 days
        return 31;
    }
}

struct tm *get_time() {
    time_t tt = time(0);
    return localtime(&tt);
}

void setColors(GContext* ctx) {
    window_set_background_color(s_window, GColorBlack);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_text_color(ctx, GColorWhite);
}

void setInvColors(GContext* ctx) {
  window_set_background_color(s_window, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_text_color(ctx, GColorBlack);
}

void update_time_text() {
  static char time_text[] = "00:00";

  char *time_format;

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, currentTime);

  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(sTimeLayer, time_text);
}

void update_date_text() {
  static char buffer[] = "00/00";
  strftime(buffer, sizeof(buffer), "%m/%d", currentTime);
  
  if (buffer[0] == '0') {
    memmove(buffer, &buffer[1], sizeof(buffer) - 1);
  }

  text_layer_set_text(sDateLayer, buffer);
  
  static char dayText[] = "\0\0\0\0\0\0\0\0\0";
  strftime(dayText, sizeof(dayText), "%A", currentTime);
  
  text_layer_set_text(sDayLayer, dayText);
}

void calendar_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  int mon = currentTime->tm_mon;
  int year = currentTime->tm_year + 1900;
  int daysLastMonth = daysInMonth(mon-1, year);
  int daysThisMonth = daysInMonth(mon, year);
  int calendarDays[21];
  
  for (int i = 0; i < 21; i++) {
    int day = currentTime->tm_mday - currentTime->tm_wday + i;
    if (day <= 0) {
      calendarDays[i] = day + daysLastMonth;
    
    }
    else if (day > daysThisMonth) {
      calendarDays[i] = day - daysThisMonth;
    }
    else {
      calendarDays[i] = day;
    }
  }
  
// ---------------------------
// Now that we've calculated which days go where, we'll move on to the display logic.
// ---------------------------

  int weeks  =  3;  // always display 3 weeks: # previous, current, # next
    
  GFont normal = fonts_get_system_font(FONT_KEY_GOTHIC_14); // fh = 16
  GFont bold   = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD); // fh = 22
  GFont current = normal;
  int font_vert_offset = 0;
  
  char daysOfWeek[7][3] = {
    "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"
  };

  // generate a light background for the calendar grid
  setInvColors(ctx);
  graphics_fill_rect(ctx, GRect (CAL_LEFT + CAL_GAP, CAL_HEIGHT - CAL_GAP, DEVICE_WIDTH - 2 * (CAL_LEFT + CAL_GAP), CAL_HEIGHT * weeks), 0, GCornerNone);
  setColors(ctx);

  for (int col = 0; col < CAL_DAYS; col++) {
  // Adjust labels by specified offset
    int weekday = col;
    if (weekday > 6) { weekday -= 7; }

    if (col == currentTime->tm_wday) {
      current = bold;
      font_vert_offset = -3;
    }

    // draw the cell text
    graphics_draw_text(ctx, daysOfWeek[weekday], current, GRect(CAL_WIDTH * col + CAL_LEFT + CAL_GAP, CAL_GAP + font_vert_offset, CAL_WIDTH, CAL_HEIGHT), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
    if (col == currentTime->tm_wday) {
      current = normal;
      font_vert_offset = 0;
    }
  }
    
  current = normal;
  font_vert_offset = 0;

  int show_last = 0;
  
  // draw the individual calendar rows/columns
  int week = 0;
  int specialRow = show_last+1;
    
  for (int row = 1; row <= 3; row++) {
    week++;
    for (int col = 0; col < CAL_DAYS; col++) {
      if ( row == specialRow && col == currentTime->tm_wday) {
        setInvColors(ctx);
        current = bold;
        font_vert_offset = -3;
      }

      // draw the cell background
      graphics_fill_rect(ctx, GRect (CAL_WIDTH * col + CAL_LEFT + CAL_GAP, CAL_HEIGHT * week, CAL_WIDTH - CAL_GAP, CAL_HEIGHT - CAL_GAP), 0, GCornerNone);

      // draw the cell text
      char date_text[3];
      snprintf(date_text, sizeof(date_text), "%d", calendarDays[col + 7 * (row - 1)]);
      graphics_draw_text(ctx, date_text, current, GRect(CAL_WIDTH * col + CAL_LEFT, CAL_HEIGHT * week - CAL_GAP + font_vert_offset, CAL_WIDTH, CAL_HEIGHT), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 

      if ( row == specialRow && col == currentTime->tm_wday) {
        setColors(ctx);
        current = normal;
        font_vert_offset = 0;
      }
    }
  }
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  *currentTime = *tick_time;
  update_time_text();

  if (units_changed & DAY_UNIT) {
    update_date_text();
    layer_mark_dirty(sCalendarLayer);
  }
}

void handle_battery(BatteryChargeState charge) {
  if (charge.is_charging && !(bitmap_layer_get_bitmap(sChargingLayer) == s_res_charging)) {
    bitmap_layer_set_bitmap(sChargingLayer, s_res_charging);
  }
  else if(!(bitmap_layer_get_bitmap(sChargingLayer) == s_res_empty)) {
    bitmap_layer_set_bitmap(sChargingLayer, s_res_empty);
  }
  
  if (charge.charge_percent >= 90 && !(bitmap_layer_get_bitmap(sBatteryLayer) == s_res_battery_100)) {
    bitmap_layer_set_bitmap(sBatteryLayer, s_res_battery_100);
  }
  else if (charge.charge_percent >= 70 && charge.charge_percent < 90 && !(bitmap_layer_get_bitmap(sBatteryLayer) == s_res_battery_80)) {
    bitmap_layer_set_bitmap(sBatteryLayer, s_res_battery_80);
  }
  else if (charge.charge_percent >= 50 && charge.charge_percent < 70 && !(bitmap_layer_get_bitmap(sBatteryLayer) == s_res_battery_60)) {
    bitmap_layer_set_bitmap(sBatteryLayer, s_res_battery_60);
  }
  else if (charge.charge_percent >= 30 && charge.charge_percent < 50 && !(bitmap_layer_get_bitmap(sBatteryLayer) == s_res_battery_40)) {
    bitmap_layer_set_bitmap(sBatteryLayer, s_res_battery_40);
  }
  else if (charge.charge_percent >= 10 && charge.charge_percent < 30 && !(bitmap_layer_get_bitmap(sBatteryLayer) == s_res_battery_20)) {
    bitmap_layer_set_bitmap(sBatteryLayer, s_res_battery_20);
  }
  else if(charge.charge_percent < 10 && !(bitmap_layer_get_bitmap(sBatteryLayer) == s_res_battery_0)) {
    bitmap_layer_set_bitmap(sBatteryLayer, s_res_battery_0);
  }

}

/*
static void handle_connection_change(void * data) {
  bool connected = bluetooth_connection_service_peek();
  if (connected == *(bool*)data) {
    if (connected == true && !(bitmap_layer_get_bitmap(sBluetoothLayer) == s_res_bluetooth)) {
      bitmap_layer_set_bitmap(sBluetoothLayer, s_res_bluetooth);
    }
    if (connected == false && !(bitmap_layer_get_bitmap(sBluetoothLayer) == s_res_empty)){
      bitmap_layer_set_bitmap(sBluetoothLayer, s_res_empty);
    }
    vibes_short_pulse();
  }
  sTimer = NULL;
}
*/
  
static void handle_bluetooth(bool connected) {
  if (connected == true && !(bitmap_layer_get_bitmap(sBluetoothLayer) == s_res_bluetooth)) {
    bitmap_layer_set_bitmap(sBluetoothLayer, s_res_bluetooth);
  }
  if (connected == false && !(bitmap_layer_get_bitmap(sBluetoothLayer) == s_res_empty)){
    bitmap_layer_set_bitmap(sBluetoothLayer, s_res_empty);
  }
  vibes_short_pulse();
}

static void handle_window_unload(Window* window) {
  destroy_ui();
  
  gbitmap_destroy(s_res_charging);
  gbitmap_destroy(s_res_battery_20);
  gbitmap_destroy(s_res_battery_40);
  gbitmap_destroy(s_res_battery_60);
  gbitmap_destroy(s_res_battery_80);
  gbitmap_destroy(s_res_battery_100);
}

void show_watchface(void) {
  initialise_ui();
  
  s_res_charging = gbitmap_create_with_resource(RESOURCE_ID_CHARGING);
  s_res_battery_20 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_20);
  s_res_battery_40 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_40);
  s_res_battery_60 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_60);
  s_res_battery_80 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_80);
  s_res_battery_100 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_100);
  
  layer_set_update_proc(sCalendarLayer, calendar_layer_update_callback);
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(handle_bluetooth);
  
  sTimer = NULL;
  
  btStatus = bluetooth_connection_service_peek();
  handle_battery(battery_state_service_peek());
  
  currentTime = get_time();
  update_time_text();
  update_date_text();
  layer_mark_dirty(sCalendarLayer);
  
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_watchface(void) {
  window_stack_remove(s_window, true);
}
