/***
 * Multi Timer
 * Copyright © 2013 Matthew Tole
 *
 * win-controls.c
 ***/

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "win-controls.h"
#include "timers.h"
#include "timer.h"
#include "bitmaps.h"

#define MENU_NUM_ROWS 4
#define MENU_ROW_START 0
#define MENU_ROW_PAUSE 1
#define MENU_ROW_RESET 2
#define MENU_ROW_CLEAR 3

static uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data);
static uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data);
static int16_t menu_get_header_height_callback(MenuLayer *me, uint16_t section_index, void *data);
static int16_t menu_get_cell_height_callback(MenuLayer* me, MenuIndex* cell_index, void* data);
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);
static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static Window* window;
static MenuLayer* layer_menu;

void win_controls_init(void) {
  window = window_create();

  layer_menu = menu_layer_create(layer_get_bounds(window_get_root_layer(window)));
  menu_layer_set_callbacks(layer_menu, NULL, (MenuLayerCallbacks) {
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_click_callback,
  });
  menu_layer_set_click_config_onto_window(layer_menu, window);
  layer_add_child(window_get_root_layer(window), menu_layer_get_layer(layer_menu));
}

void win_controls_show(void) {
  window_stack_push(window, true);
}

void win_controls_destroy(void) {
  menu_layer_destroy(layer_menu);
  window_destroy(window);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data) {
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
  return MENU_NUM_ROWS;
}

static int16_t menu_get_header_height_callback(MenuLayer *me, uint16_t section_index, void *data) {
  return 0;
}

static int16_t menu_get_cell_height_callback(MenuLayer* me, MenuIndex* cell_index, void* data) {
  return 37;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  GBitmap* icon = NULL;
  char* label = malloc(32);

  switch (cell_index->row) {
    case MENU_ROW_START:
      strcpy(label, "Start All");
      icon = bitmaps_get_bitmap(RESOURCE_ID_MENU_ICON_PLAY);
    break;
    case MENU_ROW_PAUSE:
      strcpy(label, "Pause All");
      icon = bitmaps_get_bitmap(RESOURCE_ID_MENU_ICON_PAUSE);
    break;
    case MENU_ROW_RESET:
      strcpy(label, "Reset All");
    break;
    case MENU_ROW_CLEAR:
      strcpy(label, "Clear Timers");
    break;
  }
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_text_draw(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(36, 2, 104, 24), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  if (icon != NULL) {
    graphics_draw_bitmap_in_rect(ctx, icon, GRect(8, 6, 24, 24));
  }
  free(label);
}

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  switch (cell_index->row) {
    case MENU_ROW_START:
      for (int t = 0; t < get_timer_count(); t += 1) {
        timer_start(get_timer(t));
      }
    break;
    case MENU_ROW_PAUSE:
      for (int t = 0; t < get_timer_count(); t += 1) {
        timer_pause(get_timer(t));
      }
    break;
    case MENU_ROW_RESET:
      for (int t = 0; t < get_timer_count(); t += 1) {
        timer_reset(get_timer(t));
      }
    break;
    case MENU_ROW_CLEAR:
      clear_timers();
    break;
  }
  window_stack_pop(true);
}
