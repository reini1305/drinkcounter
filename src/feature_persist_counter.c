#include "pebble.h"
# include <drink.h>
#include "autoconfig.h"
// TODOS:
// Add configuration dialog
// Make drink types 

// This is a custom defined key for saving our count field
#define NUM_BEERS_PKEY 1
#define NUM_WINE_PKEY 2
#define NUM_COCKTAILS_PKEY 3
#define NUM_SHOTS_PKEY 5
#define LAST_DRINK_TIME 4
#define FIRST_DRINK_TIME 12 // 6 to 11 are used for config!
#define NUM_CIGARETTES_PKEY 13
#define DRAWING_ORDER_PKEY 14

// You can define defaults for values in persistent storage
#define NUM_DRINKS_DEFAULT 0

// Main Window
static Window *window;

static Drink drinks[5];
static unsigned char drawing_order[5]={4,3,2,1,0};
static float sum_drinks=0;

static GBitmap *action_icon_plus;
static GBitmap *action_icon_reset;
static GBitmap *action_icon_right;

static GBitmap *status_icon_bitmap;

static ActionBarLayer *action_bar;

static AppTimer *light_timer;

static Layer *scroll_layer;
static int16_t width;
static PropertyAnimation *animation;

static TextLayer *header_text_layer;

// Confirmation Dialog
static Window *conf_dialog;
static ActionBarLayer *conf_action_bar;
static TextLayer *conf_text_layer;
static GBitmap *action_icon_cancel;
static GBitmap *action_icon_confirm;
static bool reset = false;

// We'll save the count in memory from persistent storage
static struct tm last_drink_time;
static struct tm first_drink_time;
static struct tm current_time;

static int current_drink = 0;

static void light_off(void *data)
{
  light_enable(false);
}

static void light_on(void)
{
  light_enable(true);
  if(!app_timer_reschedule(light_timer,10000))
    light_timer = app_timer_register(10000,light_off,NULL);
}

static void animation_stopped(PropertyAnimation *animation, bool finished, void *data) {
  property_animation_destroy(animation);
}

static void animate_layer_bounds(Layer* layer, GRect toRect)
{
  animation = property_animation_create_layer_frame(layer, NULL, &toRect);
  animation_set_handlers((Animation*) animation, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler) animation_stopped,
  }, NULL);
  animation_schedule((Animation*)animation);
}

static float get_ebac()
{
  sum_drinks=0;
  for (int i=0; i<4; i++)
  {
    sum_drinks+=drinks[i].num_drinks;
  }
  
  int day1 = current_time.tm_yday;
  int hour1 = current_time.tm_hour;
  int min1 = current_time.tm_min;
  long int combine1 = min1+hour1*60+day1*24*60;
  
  int day2 = first_drink_time.tm_yday;
  int hour2 = first_drink_time.tm_hour;
  int min2 = first_drink_time.tm_min;
  long int combine2 = min2+hour2*60+day2*24*60;
  
  unsigned int time_diff = abs(combine1 - combine2);

  if(getSize())
    sum_drinks+=drinks[0].num_drinks/0.33*0.5-drinks[0].num_drinks;
  
  float bw = getSex()==0 ? 0.58f:0.49f;
  float scale_factor = getUnit()==0? 1.0f:0.453592f; // 0 = kg, 1 = pounds
  float multiplication = getOutput()==0? 10.f:1.f; //0 = %o, 1 = %
  return( ((0.806f * sum_drinks * 1.2f)/(bw*(float)getWeight()*scale_factor) - (0.017f * (time_diff/60.f)))*multiplication);
}

static float max(float val1,float val2)
{
  return(val1>val2? val1:val2);
}

static void update_text() {
  static char output_text[35];
  
  for (int i=0; i<5; i++)
  {
    redrawText(&drinks[i]);
  }
  float ebac = max(get_ebac(),0.0f);
  if(sum_drinks<1.0f)
    text_layer_set_text(header_text_layer,"Drink Counter");
  else if (getEbac()==false)
    {
      int day1 = current_time.tm_yday;
      int hour1 = current_time.tm_hour;
      int min1 = current_time.tm_min;
      long int combine1 = min1+hour1*60+day1*24*60;
      
      int day2 = last_drink_time.tm_yday;
      int hour2 = last_drink_time.tm_hour;
      int min2 = last_drink_time.tm_min;
      long int combine2 = min2+hour2*60+day2*24*60;
      
      unsigned int time_diff = abs(combine1 - combine2);
      
      snprintf(output_text, sizeof(output_text), "Time since last drink: %d %s",time_diff>60?time_diff/60:time_diff,
               time_diff>60?"hour(s)":"min(s)");
      text_layer_set_text(header_text_layer,output_text);
    }
    else
    {
      snprintf(output_text,sizeof(output_text),"EBAC: %d.%03d%s\n(%s, %d%s)",(int)ebac,(int)(ebac*1000.f)-((int)ebac)*1000,
               getOutput()==0? "‰":"%",
               getSex()==0? "M":"F",(int)getWeight(),getUnit()==0?"kg":"lbs");
      text_layer_set_text(header_text_layer,output_text);
    }
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  autoconfig_in_received_handler(iter, context);
  
  update_text();
}

static void update_selection() {
  switch (current_drink) {
    case 0:
      deselectDrink(&drinks[drawing_order[4]]);
      selectDrink(&drinks[drawing_order[0]]);
      animate_layer_bounds(scroll_layer,GRect(3,64,5/3*width,100));
    break;
    case 1:
      deselectDrink(&drinks[drawing_order[0]]);
      selectDrink(&drinks[drawing_order[1]]);
    break;
    case 2:
      deselectDrink(&drinks[drawing_order[1]]);
      selectDrink(&drinks[drawing_order[2]]);
      animate_layer_bounds(scroll_layer,GRect(3-width/3,64,5/3*width,100));
    break;
    case 3:
      deselectDrink(&drinks[drawing_order[2]]);
      selectDrink(&drinks[drawing_order[3]]);
      animate_layer_bounds(scroll_layer,GRect(3-2*width/3,64,5/3*width,100));
      break;
    case 4:
      deselectDrink(&drinks[drawing_order[3]]);
      selectDrink(&drinks[drawing_order[4]]);
    break;
    default:
    break;
  }

}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  
  // Save the current time
  current_time = *tick_time;
  update_text();
}

static void reset_counters(Window *me)
{
  text_layer_destroy(conf_text_layer);
  action_bar_layer_destroy(conf_action_bar);
  if(reset)
  {
    for (int i=0;i<5;i++)
      resetCounter(&drinks[i]);
    update_text();
  }
}

static void increment_click_handler(ClickRecognizerRef recognizer, void *context) {
  light_on();
  sum_drinks=0;
  for (int i=0; i<4; i++)
  {
    sum_drinks+=drinks[i].num_drinks;
  }
  // We started drinking
  if(sum_drinks<1.0f)
    first_drink_time = current_time;
  if(current_drink!=4) // cigarette is no drink
    last_drink_time = current_time;
  increaseCounter(&drinks[drawing_order[current_drink]]);
  update_text();
}

static void conf_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  reset = true;
  window_stack_pop(true);
}

static void conf_cancel_click_handler(ClickRecognizerRef recognizer, void *context) {
  reset = false;
  window_stack_pop(true);
}

static void conf_click_config_provider(void *context) {
  const uint16_t repeat_interval_ms = 1000;
  window_single_repeating_click_subscribe(BUTTON_ID_SELECT, repeat_interval_ms, (ClickHandler) conf_select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, (ClickHandler) conf_cancel_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_BACK, repeat_interval_ms, (ClickHandler) conf_cancel_click_handler);
}

static void dialog_load(Window *me)
{
  conf_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(conf_action_bar, me);
  action_bar_layer_set_click_config_provider(conf_action_bar, conf_click_config_provider);
  
  action_bar_layer_set_icon(conf_action_bar, BUTTON_ID_SELECT, action_icon_confirm);
  action_bar_layer_set_icon(conf_action_bar, BUTTON_ID_DOWN, action_icon_cancel);

  Layer *layer = window_get_root_layer(me);
  const int16_t width = layer_get_frame(layer).size.w - ACTION_BAR_WIDTH - 6;
  
  conf_text_layer = text_layer_create(GRect(4, 20, width, 160));
  text_layer_set_font(conf_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_background_color(conf_text_layer, GColorClear);
  text_layer_set_text_alignment(conf_text_layer,GTextAlignmentCenter);
  text_layer_set_text(conf_text_layer, "Do you want to reset?");
  layer_add_child(layer, text_layer_get_layer(conf_text_layer));
  reset = false;
}

static void reset_click_handler(ClickRecognizerRef recognizer, void *context) {
  light_on();
  window_stack_push(conf_dialog,true);
}

static void immediate_reset_click_handler(ClickRecognizerRef recognizer, void *context) {
  light_on();
  for (int i=0;i<5;i++)
    resetCounter(&drinks[i]);
  update_text();
}


static void right_click_handler(ClickRecognizerRef recognizer, void *context) {
  light_on();
  current_drink++;
  if (current_drink>4) {
    current_drink=0;
  }
  update_selection();
}

static void move_right_click_handler(ClickRecognizerRef recognizer, void *context) {
  light_on();
  
  // Swap current drink with the next drink
  unsigned char next_drink = current_drink+1;
  if (next_drink>4) {
    next_drink=0;
  }
  swapDrinks(&drinks[drawing_order[current_drink]], &drinks[drawing_order[next_drink]]);
  unsigned char temp = drawing_order[current_drink];
  drawing_order[current_drink] = drawing_order[next_drink];
  drawing_order[next_drink] = temp;
  
  current_drink = next_drink;
  
  update_selection();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) reset_click_handler);
  window_long_click_subscribe(BUTTON_ID_UP,1000,(ClickHandler) immediate_reset_click_handler,NULL);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) right_click_handler);
  window_long_click_subscribe(BUTTON_ID_DOWN, 1000, (ClickHandler) move_right_click_handler,NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) increment_click_handler);
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  // Process tap on ACCEL_AXIS_X, ACCEL_AXIS_Y or ACCEL_AXIS_Z
  // Direction is 1 or -1
  light_on();
}

static void window_load(Window *me) {
  action_bar = action_bar_layer_create();
  
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
  //window_set_click_config_provider(window,window_click_config_provider);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_reset);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_right);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_plus);

  Layer *layer = window_get_root_layer(me);
  width = layer_get_frame(layer).size.w - ACTION_BAR_WIDTH - 6;

  header_text_layer = text_layer_create(GRect(3, 0, width, 60));
  text_layer_set_font(header_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_background_color(header_text_layer, GColorClear);
  text_layer_set_text_alignment(header_text_layer,GTextAlignmentCenter);
  //text_layer_set_text(header_text_layer, "Drink Counter");
  layer_add_child(layer, text_layer_get_layer(header_text_layer));

  int grid_size_v = width/3;
  
  layer_set_clips(layer,false);
  scroll_layer = layer_create(GRect(3,64,5/3*width,100));
  layer_set_clips(scroll_layer,false);
  layer_add_child(layer,scroll_layer);
  
  uint32_t ressources[5] = {RESOURCE_ID_IMAGE_BEER,RESOURCE_ID_IMAGE_WINE,RESOURCE_ID_IMAGE_COCKTAIL,RESOURCE_ID_IMAGE_SHOT,RESOURCE_ID_IMAGE_CIGARETTE};
  unsigned char storage_slots[5] = {NUM_BEERS_PKEY,NUM_WINE_PKEY,NUM_COCKTAILS_PKEY,NUM_SHOTS_PKEY,NUM_CIGARETTES_PKEY};
  
  for(int i=0;i<5;i++)
    createDrink(&drinks[i], scroll_layer, ressources[i], storage_slots[i], drawing_order[i], grid_size_v);
  
  action_bar_layer_add_to_window(action_bar, me);
  light_on();
  update_text();
  update_selection();
}

static void window_unload(Window *window) {
  text_layer_destroy(header_text_layer);

  action_bar_layer_destroy(action_bar);
  layer_destroy(scroll_layer);
  light_enable(false);
}

static void init(void) {
  autoconfig_init();
  app_message_register_inbox_received(in_received_handler);

  action_icon_plus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLUS);
  action_icon_reset = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_RESET);
  action_icon_right = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_RIGHT);
  action_icon_cancel = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_CROSS);
  action_icon_confirm = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_CHECK);
  
  light_timer = app_timer_register(10000,light_off,NULL);
  
  accel_tap_service_subscribe(&accel_tap_handler);
  
  // Get the count from persistent storage for use if it exists, otherwise use the default
  if(persist_exists(LAST_DRINK_TIME))
    persist_read_data(LAST_DRINK_TIME,&last_drink_time,sizeof(last_drink_time));
  else
    last_drink_time = current_time;
  if(persist_exists(FIRST_DRINK_TIME))
    persist_read_data(FIRST_DRINK_TIME,&first_drink_time,sizeof(first_drink_time));
  else
    first_drink_time = current_time;
  if(persist_exists(DRAWING_ORDER_PKEY))
    persist_read_data(DRAWING_ORDER_PKEY,drawing_order,5*sizeof(unsigned char));

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  conf_dialog = window_create();
  window_set_window_handlers(conf_dialog, (WindowHandlers) {
    .load = dialog_load,
    .unload = reset_counters,
  });
  
  status_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STATUS_ICON);
  window_set_status_bar_icon(window,status_icon_bitmap);

  window_stack_push(window, true /* Animated */);
}

static void deinit(void) {
  
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  
  window_destroy(window);
  window_destroy(conf_dialog);

  gbitmap_destroy(action_icon_plus);
  gbitmap_destroy(action_icon_reset);
  gbitmap_destroy(action_icon_right);
  gbitmap_destroy(status_icon_bitmap);
  gbitmap_destroy(action_icon_cancel);
  gbitmap_destroy(action_icon_confirm);
  
  for (int i=0;i<5; i++) {
    destroyDrink(&drinks[i]);
  }
  // Save the count into persistent storage on app exit
  persist_write_data(LAST_DRINK_TIME,&last_drink_time,sizeof(last_drink_time));
  persist_write_data(FIRST_DRINK_TIME,&first_drink_time,sizeof(first_drink_time));
  persist_write_data(DRAWING_ORDER_PKEY,drawing_order,sizeof(unsigned char)*5);
  autoconfig_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
