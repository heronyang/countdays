#include <pebble.h>
#include "config.h"

static Window *window;
static TextLayer *time_layer;
static Layer *circle_layer;

static int hours, minutes;

static void init();
static void init_window();
static void update_time();
static void deinit();
static void tick_handler(struct tm *, TimeUnits);
static void window_load(Window *);
static void draw_circle_layer(Layer *, GRect);
static void draw_time(Layer *, GRect);
static void layer_update_proc(Layer *, GContext *);
static void window_unload(Window *);

int main() {

    init();
    app_event_loop();
    deinit();

}

static void init() {

    init_window();
    update_time();

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

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

static void update_time() {

    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    hours = tick_time->tm_hour;
    minutes = tick_time->tm_min;

    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
            "%H:%M" : "%I:%M", tick_time);

    text_layer_set_text(time_layer, s_buffer);

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

static void deinit() {

    layer_destroy(circle_layer);
    window_destroy(window);

}

static void window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    draw_circle_layer(window_layer, bounds);
    draw_time(window_layer, bounds);

}

static void draw_circle_layer(Layer *window_layer, GRect bounds) {

    circle_layer = layer_create(bounds);
    layer_set_update_proc(circle_layer, layer_update_proc);
    layer_add_child(window_layer, circle_layer);

}

static void draw_time(Layer *window_layer, GRect bounds){

    static GFont font;

    time_layer = text_layer_create(GRect(0,
                TIME_LAYER_TOP,
                bounds.size.w,
                TIME_LAYER_HEIGHT));
    font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
    text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
    text_layer_set_font(time_layer, font);
    layer_add_child(window_layer, text_layer_get_layer(time_layer));

}

static void layer_update_proc(Layer *layer, GContext *ctx) {

    GRect bounds = layer_get_bounds(layer);
    GRect frame = grect_inset(bounds, GEdgeInsets(0));

    int start_ang = CIRCLE_ANG_START;
    int end_ang = CIRCLE_ANG_START
        + (hours * 60 + minutes)
        * (CIRCLE_ANG_END - CIRCLE_ANG_START)
        / (24 * 60);

    graphics_context_set_fill_color(ctx, FG_COLOR);
    graphics_fill_radial(ctx,
            frame,
            GOvalScaleModeFitCircle,
            CIRCLE_THICKNESS,
            DEG_TO_TRIGANGLE(start_ang),
            DEG_TO_TRIGANGLE(end_ang)
    );

}

static void window_unload(Window *window) {
    text_layer_destroy(time_layer);
}
