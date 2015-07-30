/*
 * The original source image is from:
 *   <http://openclipart.org/detail/26728/aiga-litter-disposal-by-anonymous>
 *
 * The source image was converted from an SVG into a RGB bitmap using
 * Inkscape. It has no transparency and uses only black and white as
 * colors.
 */

#include "pebble.h"

static Window *s_main_window;
static Layer *s_image_layer;
static GBitmap *s_image;
static Layer *s_image_batterie;
static GBitmap *s_bitmap_batterie;
static TextLayer *s_time_layer;
static TextLayer *s_Date_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_year_layer;

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "chargement");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void handle_seconde_tick(struct tm* tick_time, TimeUnits units_changed) {
  // Needs to be static because it's used by the system later.
  static char s_time_text[] = "00:00";
	static char s_date_text[] = "--------------------";
	static char s_year_text[] = "----";

	// Mise à jour de l'heure
  strftime(s_time_text, sizeof(s_time_text), "%R", tick_time);
	text_layer_set_text(s_time_layer, s_time_text);

	// Mise à jour de la date
	strftime(s_date_text, sizeof(s_date_text), "%A %d %B", tick_time);
  text_layer_set_text(s_Date_layer, s_date_text);
	
	// Mise à jour de l'année
	strftime(s_year_text, sizeof(s_year_text), "%Y", tick_time);
	text_layer_set_text(s_year_layer, s_year_text);
	
  handle_battery(battery_state_service_peek());
}

static void handle_bluetooth(bool connected) {
  //text_layer_set_text(s_connection_layer, connected ? "ok" : "--");
}

static void layer_update_callback(Layer *layer, GContext* ctx) {
  // We make sure the dimensions of the GRect to draw into
	
  // are equal to the size of the bitmap--otherwise the image
  // will automatically tile. Which might be what *you* want.
#ifdef PBL_PLATFORM_BASALT
  GSize image_size = gbitmap_get_bounds(s_image).size;
#else 
  GSize image_size = s_image->bounds.size;
#endif
	graphics_draw_bitmap_in_rect(ctx, s_image, GRect(0, 0, image_size.w, image_size.h));
	
#ifdef PBL_PLATFORM_BASALT
  image_size = gbitmap_get_bounds(s_bitmap_batterie).size;
#else 
  image_size = s_bitmap_batterie->bounds.size;
#endif
	graphics_draw_bitmap_in_rect(ctx, s_bitmap_batterie, GRect(12, 111, image_size.w, image_size.h));
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_frame(window_layer);

	// Layer pour le Fond static
  s_image_layer = layer_create(bounds);
  layer_set_update_proc(s_image_layer, layer_update_callback);
  layer_add_child(window_layer, s_image_layer);
	s_image = gbitmap_create_with_resource(RESOURCE_ID_FOND_VIDE);
	
	// Layer pour le logo de la batterie
	s_image_batterie = layer_create(bounds);
	layer_set_update_proc(s_image_batterie, layer_update_callback);
  layer_add_child(window_layer, s_image_batterie);
	s_bitmap_batterie = gbitmap_create_with_resource(RESOURCE_ID_ICONE_BLUETOOTH_100);
	
	// Layer pour L'heure
  s_time_layer = text_layer_create(GRect(0, 60, bounds.size.w, 44));
  text_layer_set_text_color(s_time_layer, GColorOrange);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	text_layer_set_text(s_time_layer, "---------------");

	// Layer pour la Date
	s_Date_layer = text_layer_create(GRect(2, 50, bounds.size.w, 34));
  text_layer_set_text_color(s_Date_layer, GColorWhite);
  text_layer_set_background_color(s_Date_layer, GColorClear);
  text_layer_set_font(s_Date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_Date_layer, GTextAlignmentCenter);

	// Layer pour le pourcentage de de la batterie
  s_battery_layer = text_layer_create(GRect(-72, 106, bounds.size.w, 34));
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text(s_battery_layer, "--- %");
	
  // Layer pour l'année
  s_year_layer = text_layer_create(GRect(-12, 106, bounds.size.w, 34));
  text_layer_set_text_color(s_year_layer, GColorWhite);
  text_layer_set_background_color(s_year_layer, GColorClear);
  text_layer_set_font(s_year_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_year_layer, GTextAlignmentRight);
  text_layer_set_text(s_year_layer, "----");
	
  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_seconde_tick(current_time, SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, handle_seconde_tick);
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(handle_bluetooth);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_Date_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_year_layer));
}

static void main_window_unload(Window *window) {
	tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
	
	// Destruction de layers
  text_layer_destroy(s_time_layer);
	text_layer_destroy(s_Date_layer);
  text_layer_destroy(s_battery_layer);
	text_layer_destroy(s_year_layer);
	
	gbitmap_destroy(s_image);
  layer_destroy(s_image_layer);
	
	gbitmap_destroy(s_bitmap_batterie);
  layer_destroy(s_image_batterie);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}