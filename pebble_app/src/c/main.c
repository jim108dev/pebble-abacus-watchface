#include <pebble.h>

static Layer *s_date_layer;
static Layer *s_time_layer;
static Layer *s_window_layer;

static Window *s_main_window;
static uint8_t MAX_DIGITS = 4;
static uint8_t MAX_HEAVEN_BEAD_POSITIONS = 2;
static uint8_t MAX_EARTH_BEAD_POSITIONS = 5;
static uint8_t PADDING = 1;
static uint8_t MAX_EARTH_BEADS = 4;

//https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
int ipow(int base, int exp)
{
    int result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}

static void draw_bead(GContext *ctx, GRect bounds){

  
  GRect fill_bounds = GRect(bounds.origin.x + PADDING, bounds.origin.y + PADDING, bounds.size.w - 2*PADDING, bounds.size.h - 2*PADDING);
  graphics_draw_rect(ctx, fill_bounds);

  int corner_radius = 10;
  graphics_fill_rect(ctx, fill_bounds, corner_radius, GCornersAll);
}

static void draw_heaven_bead(GContext *ctx, GRect bounds, uint16_t bead_h, uint8_t num)  {
    bool is_five_on = num >= (MAX_EARTH_BEADS + 1);
    uint16_t heaven_bead_y = bounds.origin.y + (is_five_on ? bead_h : 0);

    GRect bead_bounds = GRect(bounds.origin.x, heaven_bead_y, bounds.size.w, bead_h);
    draw_bead(ctx, bead_bounds);
}

static void draw_earth_beads(GContext *ctx, GRect bounds, uint16_t bead_h, uint8_t num)  {
    uint8_t earth_beads = num % MAX_EARTH_BEAD_POSITIONS;
    for(uint8_t i=0; i < MAX_EARTH_BEADS; i++){
      uint16_t bead_y = earth_beads > i ? bounds.origin.y + (bead_h * i): bounds.origin.y + (bead_h * (i+1));
      GRect bead_bounds = GRect(bounds.origin.x, bead_y, bounds.size.w, bead_h);
      draw_bead(ctx, bead_bounds);
    }
}

static void draw_digit(GContext *ctx, GRect bounds, uint8_t num)  {
    uint16_t bead_h = bounds.size.h / (MAX_HEAVEN_BEAD_POSITIONS + MAX_EARTH_BEAD_POSITIONS + 1);
    GRect heaven_bounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, bead_h * MAX_HEAVEN_BEAD_POSITIONS);
    draw_heaven_bead(ctx, heaven_bounds, bead_h, num );

    GPoint start = GPoint(bounds.origin.x, bounds.origin.y + bead_h * MAX_HEAVEN_BEAD_POSITIONS + bead_h/2);
    GPoint end = GPoint(bounds.origin.x + bounds.size.w, bounds.origin.y + bead_h * MAX_HEAVEN_BEAD_POSITIONS + bead_h/2);

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx, start, end);
    graphics_context_set_stroke_color(ctx, GColorBlack);

    GRect earth_bounds = GRect(bounds.origin.x, bounds.origin.y + heaven_bounds.size.h + bead_h, bounds.size.w, bead_h * (MAX_EARTH_BEAD_POSITIONS + 1));
    draw_earth_beads(ctx, earth_bounds, bead_h, num );
}

static void draw_digits(Layer *layer, GContext *ctx, uint16_t num)  {
    GRect bounds = layer_get_unobstructed_bounds(layer);
    uint16_t digit_w = bounds.size.w / MAX_DIGITS; 

    for(uint8_t i=0; i < MAX_DIGITS; i++){
      uint8_t digit_num = num / ipow(10,i) % 10;
      GRect digit_bounds = GRect(bounds.origin.x + (MAX_DIGITS - i - 1) * digit_w, bounds.origin.y, digit_w, bounds.size.h);
      draw_digit(ctx, digit_bounds, digit_num);
    }
}

static void update_date(Layer *layer, GContext *ctx)  {
      
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  graphics_context_set_fill_color(ctx, GColorWhite);

  draw_digits(layer, ctx, tick_time->tm_mday * ipow(10,2) + tick_time->tm_mon + 1);
}

static void update_time(Layer *layer, GContext *ctx)  {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  graphics_context_set_fill_color(ctx, GColorWhite);

  draw_digits(layer, ctx, tick_time->tm_hour * ipow(10,2) + tick_time->tm_min);
}

static void date_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    layer_mark_dirty(s_date_layer);
}
static void time_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    layer_mark_dirty(s_time_layer);
}

static void main_window_load(Window *window) {
    window_set_background_color(window, GColorBlack);
    
    s_window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(s_window_layer);

    int16_t box_h = bounds.size.h/2 - 4;
    s_date_layer = layer_create(GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, box_h ));
    s_time_layer = layer_create(GRect(bounds.origin.x, bounds.origin.y + box_h + 8 , bounds.size.w, box_h ));
    
    layer_add_child(s_window_layer, s_date_layer);
    layer_add_child(s_window_layer, s_time_layer);

    layer_set_update_proc(s_date_layer, &update_date);
    layer_set_update_proc(s_time_layer, &update_time);
}

static void main_window_unload(Window *window) {
  layer_destroy(s_date_layer);
  layer_destroy(s_time_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  tick_timer_service_subscribe(DAY_UNIT, date_tick_handler);
  tick_timer_service_subscribe(MINUTE_UNIT, time_tick_handler);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}