#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_hour_layer;
static TextLayer *s_minute_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static GFont s_time_font;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_hour_buffer[8];
  static char s_minute_buffer[8];
  strftime(s_hour_buffer, sizeof(s_hour_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);
  strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_hour_layer, s_hour_buffer);
  text_layer_set_text(s_minute_layer, s_minute_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  
  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);
  
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // Create the TextLayers with specific bounds
  // parent text layer
  s_time_layer = text_layer_create( GRect(0, 0, bounds.size.w, 168));
  // children
  s_hour_layer = text_layer_create( GRect(4, 26, bounds.size.w, 168));
  s_minute_layer = text_layer_create( GRect(4, 98, bounds.size.w, 168));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorBlack);
  text_layer_set_text(s_hour_layer, "00");
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  
  text_layer_set_background_color(s_minute_layer, GColorClear);
  text_layer_set_text_color(s_minute_layer, GColorBlack);
  text_layer_set_text(s_minute_layer, "00");
  text_layer_set_text_alignment(s_minute_layer, GTextAlignmentCenter);

  // Add parent text as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Add hour and minute layers to parent text
  layer_add_child(text_layer_get_layer(s_time_layer), text_layer_get_layer(s_hour_layer));
  layer_add_child(text_layer_get_layer(s_time_layer), text_layer_get_layer(s_minute_layer));
  
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ZU_47));
  
  // Apply to TextLayers
  text_layer_set_font(s_hour_layer, s_time_font);
  text_layer_set_font(s_minute_layer, s_time_font);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_minute_layer);
  
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}

void handle_init(void) {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();
  
  window_set_background_color(s_main_window, GColorBlack);
}

void handle_deinit(void) {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
