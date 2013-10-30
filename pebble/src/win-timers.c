/***
 * Multi Timer
 * Copyright © 2013 Matthew Tole
 *
 * win-timers.c
 ***/

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>
#include "timers.h"
#include "timer.h"
#include "bitmaps.h"
#include "win-add.h"
#include "win-controls.h"
#include "win-settings.h"

#define MENU_SECTIONS 2
#define MENU_SECTION_TIMERS 0
#define MENU_SECTION_FOOTER 1

#define MENU_ROWS_FOOTER 3

#define MENU_ITEM_FOOTER_ADD 0
#define MENU_ITEM_FOOTER_CONTROLS 1
#define MENU_ITEM_FOOTER_SETTINGS 2

#define ROW_HEIGHT 36

static void window_appear(Window* window);
static uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data);
static uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data);
static int16_t menu_get_header_height_callback(MenuLayer *me, uint16_t section_index, void *data);
static int16_t menu_get_cell_height_callback(MenuLayer* me, MenuIndex* cell_index, void* data);
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data);
static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);
static void menu_draw_footer_row(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);
static void menu_draw_timer_row(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);
static void jump_to_timer(int t, bool animate);
static void show_add_timer();

static Window* window;
static MenuLayer* layer_menu;

void win_timers_init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .appear = window_appear
  });

  layer_menu = menu_layer_create(layer_get_bounds(window_get_root_layer(window)));
  menu_layer_set_callbacks(layer_menu, NULL, (MenuLayerCallbacks) {
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_click_callback,
    .select_long_click = menu_select_long_click_callback,
  });
  menu_layer_set_click_config_onto_window(layer_menu, window);
  layer_add_child(window_get_root_layer(window), menu_layer_get_layer(layer_menu));

  win_add_init();
  win_controls_init();
  win_settings_init();
}

void win_timers_show(void) {
  window_stack_push(window, true);
}

void win_timers_update(void) {
  menu_layer_reload_data(layer_menu);
}

void win_timers_destroy(void) {
  win_add_destroy();
  win_controls_destroy();
  win_settings_destroy();
  window_destroy(window);
  menu_layer_destroy(layer_menu);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void window_appear(Window* window) {
  menu_layer_reload_data(layer_menu);
}

static uint16_t menu_get_num_sections_callback(MenuLayer *me, void *data) {
  return MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *me, uint16_t section_index, void *data) {
  switch (section_index) {
    case MENU_SECTION_FOOTER:
      return MENU_ROWS_FOOTER;
    break;
    case MENU_SECTION_TIMERS:
      return get_timer_count();
    break;
  }
  return 0;
}

static int16_t menu_get_header_height_callback(MenuLayer *me, uint16_t section_index, void *data) {
  if (get_timer_count() == 0) {
    return 0;
  }

  switch (section_index) {
    case MENU_SECTION_FOOTER:
      return 4;
    break;
    case MENU_SECTION_TIMERS:
      return 0;
    break;
  }
  return 0;
}

static int16_t menu_get_cell_height_callback(MenuLayer* me, MenuIndex* cell_index, void* data) {
  switch (cell_index->section) {
    case MENU_SECTION_TIMERS:
      return ROW_HEIGHT;
    break;
    case MENU_SECTION_FOOTER:
      return ROW_HEIGHT + 4;
    break;
  }
  return 0;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case MENU_SECTION_FOOTER:
    break;
    case MENU_SECTION_TIMERS:
    break;
  }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case MENU_SECTION_FOOTER:
      menu_draw_footer_row(ctx, cell_layer, cell_index, data);
    break;
    case MENU_SECTION_TIMERS:
      menu_draw_timer_row(ctx, cell_layer, cell_index, data);
    break;
  }
}

static void menu_draw_footer_row(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  char* row_label = malloc(20);
  GBitmap* row_icon = NULL;

  switch (cell_index->row) {
    case MENU_ITEM_FOOTER_ADD:
      strcpy(row_label, "Add Timer");
      row_icon = bitmaps_get_bitmap(RESOURCE_ID_MENU_ICON_ADD);
    break;
    case MENU_ITEM_FOOTER_CONTROLS:
      strcpy(row_label, "Controls");
      row_icon = bitmaps_get_bitmap(RESOURCE_ID_MENU_ICON_CONTROLS);
    break;
    case MENU_ITEM_FOOTER_SETTINGS:
      strcpy(row_label, "Settings");
      row_icon = bitmaps_get_bitmap(RESOURCE_ID_MENU_ICON_SETTINGS);
    break;
  }

  graphics_context_set_text_color(ctx, GColorBlack);
  if (row_icon != NULL) {
    graphics_draw_bitmap_in_rect(ctx, row_icon, GRect(4, 8, 24, 24));
  }
  graphics_text_draw(ctx, row_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(32, 3, 112, 24), 0, GTextAlignmentLeft, NULL);
}

static void menu_draw_timer_row(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  Timer* timer = get_timer(cell_index->row);
  char* time_left = malloc(32);
  GBitmap* row_bmp = NULL;

  if (timer == NULL) {
    return;
  }
  snprintf(time_left, 32, "%02d:%02d", timer->time_left / 60, timer->time_left % 60);
  switch (timer->status) {
    case TIMER_RUNNING:
      row_bmp = bitmaps_get_bitmap(RESOURCE_ID_MENU_ICON_PLAY);
    break;
    case TIMER_PAUSED:
      row_bmp = bitmaps_get_bitmap(RESOURCE_ID_MENU_ICON_PAUSE);
    break;
    case TIMER_STOPPED:
      row_bmp = bitmaps_get_bitmap(RESOURCE_ID_MENU_ICON_STOP);
    break;
    case TIMER_FINISHED:
      row_bmp = bitmaps_get_bitmap(RESOURCE_ID_MENU_ICON_DONE);
    break;
  }

  graphics_context_set_text_color(ctx, GColorBlack);
  if (row_bmp != NULL) {
    graphics_draw_bitmap_in_rect(ctx, row_bmp, GRect(4, 6, 24, 24));
  }
  graphics_text_draw(ctx, time_left, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(32, 2, 110, 24), 0, GTextAlignmentLeft, NULL);

  free(time_left);
}

static void menu_select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  switch (cell_index->section) {
    case MENU_SECTION_FOOTER: {
      switch (cell_index->row) {
        case MENU_ITEM_FOOTER_ADD:
          win_add_show();
        break;
        case MENU_ITEM_FOOTER_CONTROLS:
          win_controls_show();
        break;
        case MENU_ITEM_FOOTER_SETTINGS:
          win_settings_show();
        break;
      }
    }
    break;
    case MENU_SECTION_TIMERS: {
      if (get_timer_count() == 0) {
        return;
      }

      Timer* timer = get_timer(cell_index->row);
      if (! timer) {
        return;
      }
      switch (timer->status) {
        case TIMER_RUNNING:
          timer_pause(timer);
        break;
        case TIMER_PAUSED:
          timer_resume(timer);
        break;
        case TIMER_STOPPED:
          timer_start(timer);
        break;
        case TIMER_FINISHED:
          timer_reset(timer);
        break;
      }
    }
    break;
  }
  menu_layer_reload_data(layer_menu);
}

static void menu_select_long_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  if (cell_index->section != MENU_SECTION_TIMERS) {
    return;
  }
  Timer* timer = get_timer(cell_index->row);
  if (! timer) {
    return;
  }
  switch (timer->status) {
    case TIMER_PAUSED:
      timer_reset(timer);
    break;
    case TIMER_STOPPED:
    case TIMER_FINISHED:
      remove_timer(cell_index->row);
    break;
    default:
      return;
  }
  menu_layer_reload_data(layer_menu);
}

static void jump_to_timer(int t, bool animate) {
  MenuIndex index =  { MENU_SECTION_TIMERS, t };
  menu_layer_set_selected_index(layer_menu, index, MenuRowAlignCenter, animate);
}