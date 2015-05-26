#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
  
/* Allows display of a new screen that slides into watch view */
/* LEARNT: compilation failed because variable below wasn't in global scope */
static Window *s_main_window;  

/* First text element added to our Window */
static TextLayer *s_time_layer;

/* New text layer needed to display current weather condition/temperatures */
static TextLayer *s_weather_layer;

/* Declare our custom uploaded font from the Resources */
static GFont s_time_font;
static GFont s_weather_font;

/* Declare two more points, each for bit map and bit map layer */
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

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

/* Handler functions to manage Window's sub-elements */
static void main_window_load (Window *window) {
  /* Create GBitmap, then set to created BitmapLayer and add it as a child of main Window */
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  /* Creating TextLayer to display time */
  s_time_layer = text_layer_create(GRect(5, 52, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  
  /* Create GFont */
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));
  
  /* Apply to TextLayer */
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  /* Add it as child layer to Window's root layer */
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  /* Create temperature layer */
  s_weather_layer = text_layer_create(GRect(0, 130, 144, 25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  
  /* Create second custom font, apply it and add to Window */
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
  text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
  /* Make sure the time is displayed from the beginning */
  update_time();
}

static void main_window_unload (Window *window) {
  /* Unload GFont safely */
  fonts_unload_custom_font(s_time_font);  
  
  /* Destroy GBitmap and BitmapLayer */
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  
  /* Destroy TextLayer to keep management within loading/unloading of Window */
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_weather_layer);
  fonts_unload_custom_font(s_weather_font);
} 

/* Used to access current time by subscribing a function to run whenever time changes */
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  /* Get weather update every 30 minutes */
  /* THOUGHT: wow this is a pretty awesome way to handle frequent updates */
  if (tick_time->tm_min % 30 == 0) {
    /* Begin dictionary */
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    /* Add a key-value pair */
    dict_write_uint8(iter, 0, 0);
    
    /* Send the message! */
    app_message_outbox_send();
  }
}

/* Place callback before they are referred to is a good practice */
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  /* Store incoming information into buffers, generous with size to prevent overruns */
  /* THOUGHT: is there a better/safer way than hypothesizing the buffer size? */
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  /* Read first item */
  Tuple *t = dict_read_first(iterator);
  
  /* For all items */
  while( t != NULL ) {
    /* See which key ws received */
    switch(t->key) {
      case KEY_TEMPERATURE:
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
        break;
      case KEY_CONDITIONS:
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }
    
    /* Look for next item */
    t = dict_read_next(iterator);
  }
  
    /* Assemble the complete string and instruct TextLayer to display it */
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

/* Three additional callbacks for all possible outcomes or errors that may occur */
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void*context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

/* Helper functions to organize creation/destruction of all Pebble SDK elements */
static void init () {
  /* Create main Window element and assign to pointer */
  s_main_window = window_create(); 
  
  /* Set handlers to manage the elements inside the Window */
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  /* Display window on the watch, second parameter is animated=true */
  window_stack_push(s_main_window, true);
  
  /* Register with TickTimerService */
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler); 
  
  /* Register callbacks with the system */
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  /* Open AppMessage to allow watchface to receive incoming messages */
  /* LEARNT: Register callbacks before opening AppMessage to ensure no messages are missed */
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
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