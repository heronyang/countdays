#include "pebble.h"
#include "config.h"

static Window *window;

static TextLayer *time_layer;
static TextLayer *date_layer;
static TextLayer *countdays_layer;
static TextLayer *weather_layer;
static TextLayer *temperature_layer;

static Layer *circle_layer;
static Layer *circle_bk_layer;

static int isJustInited = true;
static int hours, minutes;

static char time_buffer[8];
static char date_buffer[8];

static char dreamday_buffer[64];

char *translate_error(AppMessageResult result);
static void init();
static void init_callbacks();
static void init_window();
static void init_countdays();
static void update_time(struct tm *);
static void deinit();
static void tick_handler(struct tm *, TimeUnits);
static int isStartOfADay(struct tm *);
static void update_date(struct tm *);
static void update_countdays_and_weather();
static void window_load(Window *);
static void draw_circle_layer(Layer *, GRect);
static void draw_circle_bk_layer(Layer *, GRect);
static void layer_update_proc(Layer *, GContext *);
static void draw_time(Layer *, GRect);
static void draw_date(Layer *, GRect);
static void draw_date_block(Layer *);
static void draw_countdays(Layer *, GRect);
static void draw_weather(Layer *, GRect);
static void draw_temperature(Layer *, GRect);
static void draw_countdays_block(Layer *);
static void window_unload(Window *);

enum {
    KEY_TEMPERATURE = 0,
    KEY_CONDITIONS = 1,
    KEY_COUNTDAYS = 2,
    KEY_DREAMDAY = 3
};

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

    Tuple *temperature = dict_find(iterator, KEY_TEMPERATURE);
    Tuple *conditions = dict_find(iterator, KEY_CONDITIONS);
    Tuple *countdays = dict_find(iterator, KEY_COUNTDAYS);
    Tuple *dreamday = dict_find(iterator, KEY_DREAMDAY);

    if (temperature && strlen(temperature->value->cstring) != 0) {
        text_layer_set_text(temperature_layer, (char *)temperature->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Get temperature: %s", (char *)temperature->value->cstring);
    } else {
        APP_LOG(APP_LOG_LEVEL_INFO, "KEY_TEMPERATURE not received.");
    }

    if (conditions && strlen(conditions->value->cstring) != 0) {
        text_layer_set_text(weather_layer, (char *)conditions->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Get conditions: %s", (char *)conditions->value->cstring);
    } else {
        APP_LOG(APP_LOG_LEVEL_INFO, "KEY_CONDITIONS not received.");
    }

    if (countdays && strlen(countdays->value->cstring) != 0) {
        text_layer_set_text(countdays_layer, (char *)countdays->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Get countdays: %s (%d)", (char *)countdays->value->cstring, strlen(countdays->value->cstring));
    } else {
        APP_LOG(APP_LOG_LEVEL_INFO, "KEY_COUNTDAYS not received.");
    }

    if (dreamday && strlen(dreamday->value->cstring) != 0) {
        snprintf(dreamday_buffer,
                sizeof(dreamday_buffer),
                "%s",
                dreamday->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Get dreamday: %s", (char *)dreamday->value->cstring);
    } else {
        APP_LOG(APP_LOG_LEVEL_INFO, "KEY_DREAMDAY not received.");
    }

}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

int main(void) {

    init();
    app_event_loop();
    deinit();

}

static void init() {

    init_window();
    init_callbacks();
    init_countdays();

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "init");

}

static void init_callbacks() {

    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    app_message_open(APP_MESSAGE_INBOX_SIZE, APP_MESSAGE_OUTBOX_SIZE);

}

static void init_window() {

    window = window_create();
    window_set_background_color(window, BG_COLOR);
    window_set_window_handlers(window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload,
            });
    window_stack_push(window, WINDOW_STACK_PUSH_ANIMATED);

}

static void init_countdays() {

    // persist_delete(DREAMDAY_KEY);

    if (persist_exists(DREAMDAY_KEY)) {

        persist_read_string(DREAMDAY_KEY,
                dreamday_buffer,
                sizeof(dreamday_buffer));
        APP_LOG(APP_LOG_LEVEL_DEBUG, "restore countdays start date");

    } else {

        snprintf(dreamday_buffer,
                sizeof(dreamday_buffer),
                DREAMDAY_DEFAULT);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "use default countdays start date");

    }

    APP_LOG(APP_LOG_LEVEL_DEBUG, "dreamday_buffer = %s", dreamday_buffer);

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {

    update_time(tick_time);

    if(isJustInited) {
        update_date(tick_time);
        update_countdays_and_weather();
        isJustInited = false;
    }

    if(isStartOfADay(tick_time)) {
        update_date(tick_time);
    }

    if(tick_time->tm_min % WEATHER_GET_EVERY_MINUTES == 0) {
        update_countdays_and_weather();
    }

}

static int isStartOfADay(struct tm *tick_time) {
    return tick_time->tm_hour == 0 && tick_time->tm_min == 0;
}

static void update_time(struct tm *tick_time) {

    hours = tick_time->tm_hour;
    minutes = tick_time->tm_min;

    strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ?
            "%H:%M" : "%I:%M", tick_time);

    text_layer_set_text(time_layer, time_buffer);

}

static void update_date(struct tm *tick_time) {

    strftime(date_buffer, sizeof(date_buffer), "%d", tick_time);

    text_layer_set_text(date_layer, date_buffer);

}

static void update_countdays_and_weather() {

    APP_LOG(APP_LOG_LEVEL_DEBUG, "update countdays");

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (!iter)  return;

    dict_write_cstring(iter, KEY_DREAMDAY,
            dreamday_buffer);
    dict_write_end(iter);

    app_message_outbox_send();

}

static void deinit() {

    layer_destroy(circle_layer);
    layer_destroy(circle_bk_layer);
    window_destroy(window);

    persist_write_string(DREAMDAY_KEY,
            dreamday_buffer);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "deinit");

}

static void window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    draw_time(window_layer, bounds);
    draw_date(window_layer, bounds);
    draw_countdays(window_layer, bounds);
    draw_weather(window_layer, bounds);
    draw_temperature(window_layer, bounds);
    draw_circle_layer(window_layer, bounds);
    draw_circle_bk_layer(window_layer, bounds);

}

static void draw_circle_layer(Layer *window_layer, GRect bounds) {

    circle_layer = layer_create(bounds);
    layer_set_update_proc(circle_layer, layer_update_proc);
    layer_add_child(window_layer, circle_layer);

}

static void draw_circle_bk_layer(Layer *window_layer, GRect bounds) {

    circle_bk_layer = layer_create(bounds);
    layer_add_child(window_layer, circle_bk_layer);

}

static void layer_update_proc(Layer *layer, GContext *ctx) {

    int start_ang = CIRCLE_ANG_START;
    int end_ang = CIRCLE_ANG_START
        + (hours * 60 + minutes)
        * (CIRCLE_ANG_END - CIRCLE_ANG_START)
        / (24 * 60);

    GRect bounds = layer_get_bounds(layer);
    GRect frame = grect_inset(bounds, GEdgeInsets(0));

    graphics_context_set_fill_color(ctx, CIRCLE_FG_COLOR);
    graphics_fill_radial(ctx,
            frame,
            GOvalScaleModeFitCircle,
            CIRCLE_THICKNESS,
            DEG_TO_TRIGANGLE(start_ang),
            DEG_TO_TRIGANGLE(end_ang)
    );

    GRect bounds_bk = layer_get_bounds(circle_bk_layer);
    GRect frame_bk = grect_inset(bounds_bk, GEdgeInsets(0));

    graphics_context_set_fill_color(ctx, CIRCLE_BG_COLOR);
    graphics_fill_radial(ctx,
            frame_bk,
            GOvalScaleModeFitCircle,
            CIRCLE_THICKNESS,
            DEG_TO_TRIGANGLE(end_ang),
            DEG_TO_TRIGANGLE(start_ang + 360)
    );

}

static void draw_time(Layer *window_layer, GRect bounds){

    static GFont font;

    time_layer = text_layer_create(GRect(0,
                TIME_LAYER_TOP,
                bounds.size.w,
                TIME_LAYER_HEIGHT));
    font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
    text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
    text_layer_set_font(time_layer, font);
    text_layer_set_background_color(time_layer, TIME_LAYER_BK_COLOR);
    text_layer_set_text_color(time_layer, TIME_LAYER_FG_COLOR);
    layer_add_child(window_layer, text_layer_get_layer(time_layer));

}

static void draw_date(Layer *window_layer, GRect bounds) {

    static GFont font;

    draw_date_block(window_layer);

    date_layer = text_layer_create(GRect(DATE_LAYER_LEFT + DATE_LAYER_LEFT_SHIFT,
                DATE_LAYER_TOP,
                DATE_WIDTH,
                DATE_HEIGHT));
    font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
    text_layer_set_text_alignment(date_layer, GTextAlignmentLeft);
    text_layer_set_font(date_layer, font);
    text_layer_set_background_color(date_layer, DATE_LAYER_BK_COLOR);
    text_layer_set_text_color(date_layer, DATE_LAYER_FG_COLOR);
    layer_add_child(window_layer, text_layer_get_layer(date_layer));

}

static void draw_date_block(Layer *window_layer) {

    TextLayer *block = text_layer_create(GRect(DATE_LAYER_LEFT,
                0,
                DATE_WIDTH,
                DATE_LAYER_TOP + DATE_HEIGHT));
    text_layer_set_background_color(block, DATE_LAYER_BK_COLOR);
    layer_add_child(window_layer, text_layer_get_layer(block));

}

static void draw_countdays(Layer *window_layer, GRect bounds) {

    draw_countdays_block(window_layer);

    static GFont font;

    countdays_layer = text_layer_create(GRect(COUNTDAYS_LAYER_LEFT,
                COUNTDAYS_LAYER_TOP,
                COUNTDAYS_LAYER_WIDTH,
                COUNTDAYS_LAYER_HEIGHT));
    font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
    text_layer_set_text_alignment(countdays_layer, GTextAlignmentLeft);
    text_layer_set_font(countdays_layer, font);
    text_layer_set_background_color(countdays_layer, COUNTDAYS_LAYER_BK_COLOR);
    text_layer_set_text_color(countdays_layer, COUNTDAYS_LAYER_FG_COLOR);
    layer_add_child(window_layer, text_layer_get_layer(countdays_layer));

}

static void draw_weather(Layer *window_layer, GRect bounds) {

    static GFont font;

    weather_layer = text_layer_create(GRect(0,
                WEATHER_LAYER_TOP,
                bounds.size.w,
                WEATHER_LAYER_HEIGHT));
    font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    text_layer_set_text_alignment(weather_layer, GTextAlignmentCenter);
    text_layer_set_font(weather_layer, font);
    text_layer_set_background_color(weather_layer, WEATHER_LAYER_BK_COLOR);
    text_layer_set_text_color(weather_layer, WEATHER_LAYER_FG_COLOR);
    layer_add_child(window_layer, text_layer_get_layer(weather_layer));

}

static void draw_temperature(Layer *window_layer, GRect bounds) {

    static GFont font;

    temperature_layer = text_layer_create(GRect(0,
                TEMPERATURE_LAYER_TOP,
                bounds.size.w,
                TEMPERATURE_LAYER_HEIGHT));
    font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
    text_layer_set_text_alignment(temperature_layer, GTextAlignmentCenter);
    text_layer_set_font(temperature_layer, font);
    text_layer_set_background_color(temperature_layer, TEMPERATURE_LAYER_BK_COLOR);
    text_layer_set_text_color(temperature_layer, TEMPERATURE_LAYER_FG_COLOR);
    layer_add_child(window_layer, text_layer_get_layer(temperature_layer));

}

static void draw_countdays_block(Layer *window_layer) {

    TextLayer *block = text_layer_create(GRect(0,
                0,
                COUNTDAYS_LAYER_LEFT + COUNTDAYS_LAYER_WIDTH,
                COUNTDAYS_LAYER_TOP + COUNTDAYS_LAYER_HEIGHT));
    text_layer_set_background_color(block, COUNTDAYS_LAYER_BK_COLOR);
    layer_add_child(window_layer, text_layer_get_layer(block));

}

static void window_unload(Window *window) {

    text_layer_destroy(time_layer);
    text_layer_destroy(date_layer);
    text_layer_destroy(weather_layer);
    text_layer_destroy(temperature_layer);
    text_layer_destroy(countdays_layer);

}
