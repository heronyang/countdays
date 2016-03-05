#pragma once

#include <pebble.h>

#define FG_COLOR GColorWhite
#define BG_COLOR GColorDarkGray

#define CIRCLE_FG_COLOR GColorBlack
#define CIRCLE_BG_COLOR GColorDarkGray

#define TIME_LAYER_BK_COLOR GColorBlack
#define TIME_LAYER_FG_COLOR GColorWhite

#define DATE_LAYER_BK_COLOR GColorMidnightGreen
#define DATE_LAYER_FG_COLOR GColorTiffanyBlue

#define WEATHER_LAYER_BK_COLOR GColorDarkGray
#define WEATHER_LAYER_FG_COLOR GColorBlack

#define TEMPERATURE_LAYER_BK_COLOR GColorCadetBlue

#define COUNTDAYS_LAYER_BK_COLOR GColorOxfordBlue
#define COUNTDAYS_LAYER_FG_COLOR GColorBlueMoon


#define CIRCLE_THICKNESS 10
#define CIRCLE_ANG_START -180
#define CIRCLE_ANG_END 180

#define WINDOW_STACK_PUSH_ANIMATED true

#define TIME_LAYER_TOP 63
#define TIME_LAYER_HEIGHT 55

#define COUNTDAYS_LAYER_TOP 30
#define COUNTDAYS_WIDTH 90
#define COUNTDAYS_HEIGHT 33

#define DATE_LAYER_LEFT 90
#define DATE_LAYER_TOP 30
#define DATE_WIDTH 90
#define DATE_HEIGHT 33
#define DATE_LAYER_LEFT_SHIFT 3

#define WEATHER_LAYER_TOP 118
#define WEATHER_LAYER_HEIGHT 33

#define APP_MESSAGE_INBOX_SIZE 128
#define APP_MESSAGE_OUTBOX_SIZE 128

#define WEATHER_GET_EVERY_MINUTES 30

#define COUNTDAYS_START_DATE_KEY 5
#define COUNTDAYS_START_DATE_DEFAULT "2016-02-29T16:00:00.000Z"
