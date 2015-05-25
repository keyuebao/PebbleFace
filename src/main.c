#include <pebble.h>

/* Allows display of a new screen that slides into watch view */
/* LEARNT: compilation failed because variable below wasn't in global scope */
static Window *s_main_window;  

/* First text element added to our Window */
static TextLayer *s_time_layer;

/* Declare our custom uploaded font from the Resources */
static GFont s_time_font;

/* Update the time TextLayer, call it from TickHandler and main_windw_load */
/* LEARNT: function order matters, as in the one gets called must be declared earlier */
static void update_time() {
  /* Get a tm structure */
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  /* Create a long-lived buffer and 24/12 hour formats */
  static char buffer[] = "00:00";
  
  /* Write current hours and minuts into the char buffer */
  if (clock_is_24h_style() == true) {
    /* Extract hours and minutes from struct tm and write into buffer */
    strftime(buffer, sizeof(buffer), "%H:%M", tick_time);
  } else {
    /* 12 hour format if could not use 24 hour style */
    strftime(buffer, sizeof(buffer), "%I:%M", tick_time);
  }
  
  /* Display the time on the TextLayer */
  text_layer_set_text(s_time_layer, buffer);
}

/* Used to access current time by subscribing a function to run whenever time changes */
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

/* Handler functions to manage Window's sub-elements */
static void main_window_load (Window *window) {
  /* Create and apply the custom font to TextLayer */
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));
  text_layer_set_font(s_time_layer, s_time_font);
  
  /* Creating TextLayer to display time */
  s_time_layer = text_layer_create(GRect(5, 52, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  /* Improve the layout to be more like a watchface */
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  /* Add it as a child layer to the Window's root layer */
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}

static void main_window_unload (Window *window) {
  /* Destroy TextLayer to keep management within loading/unloading of Window */
  text_layer_destroy(s_time_layer);
  
  /* Unload GFont safely */
  fonts_unload_custom_font(s_time_font);
} 

/* Helper functions to organize creation/destruction of all Pebble SDK elements */
static void init () {
  /* Create main Window element and assign to pointer */
  s_main_window = window_create();
  
  /* Register with TickTimerService */
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);  
  
  /* Set handlers to manage the elements inside the Window */
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  /* Display window on the watch, second parameter is animated=true */
  window_stack_push(s_main_window, true);
  
  /* Ensure the time is displayed correctly when watchface loads */
  update_time();
}

static void deinit () {
  /* Destroy window ensuring memory used is given back to the system when app exits */
  window_destroy(s_main_window);
}

int main (void) {
  init();
  
  /* Lets the watchapp wait for system event until it exits */
  app_event_loop();
  
  deinit();
}