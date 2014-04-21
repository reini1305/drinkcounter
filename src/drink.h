//
//  drink.h
//  drink_counter
//
//  Created by Christian Reinbacher on 30.03.14.
//
//

#ifndef drink_counter_drink_h
#define drink_counter_drink_h

#include "pebble.h"

typedef struct Drink{
  GBitmap* bitmap;
  BitmapLayer* bitmap_layer;
  InverterLayer* inverter_layer;
  TextLayer* text_layer;
  int num_drinks;
  unsigned char storage_slot;
  char text[3];
}Drink;

void loadDrink(Drink *drink)
{
  drink->num_drinks = persist_exists(drink->storage_slot) ? persist_read_int(drink->storage_slot) : 0;
}

void saveDrink(Drink *drink)
{
  persist_write_int(drink->storage_slot, drink->num_drinks);
}

void createDrink(Drink* drink, Layer* parent_layer, uint32_t bitmap_id, unsigned char storage_id, int position_x, int grid_size_v)
{
  int text_y = 50;
  int text_height = 100;
  
  drink->text_layer = text_layer_create(GRect(position_x, text_y, grid_size_v, text_height));
  text_layer_set_font(drink->text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_30)));
  //text_layer_set_font(drink->text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(drink->text_layer, GColorClear);
  text_layer_set_text_alignment(drink->text_layer,GTextAlignmentCenter);
  layer_add_child(parent_layer, text_layer_get_layer(drink->text_layer));
  
  drink->bitmap = gbitmap_create_with_resource(bitmap_id);
  drink->bitmap_layer = bitmap_layer_create(GRect(position_x, 0, grid_size_v, 40));
  bitmap_layer_set_alignment(drink->bitmap_layer,GAlignCenter);
  bitmap_layer_set_bitmap(drink->bitmap_layer,drink->bitmap);
  layer_add_child(parent_layer,bitmap_layer_get_layer(drink->bitmap_layer));
  
  drink->inverter_layer = inverter_layer_create(GRect(position_x, text_y, grid_size_v, text_height));
  layer_set_hidden(inverter_layer_get_layer(drink->inverter_layer),true);
  layer_add_child(parent_layer,inverter_layer_get_layer(drink->inverter_layer));
  drink->storage_slot = storage_id;
  loadDrink(drink);
}

void destroyDrink(Drink *drink)
{
  saveDrink(drink);
  text_layer_destroy(drink->text_layer);
  inverter_layer_destroy(drink->inverter_layer);
  bitmap_layer_destroy(drink->bitmap_layer);
  gbitmap_destroy(drink->bitmap);
}
void selectDrink(Drink* drink)
{
  layer_set_hidden(inverter_layer_get_layer(drink->inverter_layer),false);
}
void deselectDrink(Drink* drink)
{
  layer_set_hidden(inverter_layer_get_layer(drink->inverter_layer),true);
}

void redrawText(Drink* drink)
{
  snprintf(drink->text, sizeof(drink->text), "%u", drink->num_drinks);
  text_layer_set_text(drink->text_layer, drink->text);
}
void increaseCounter(Drink* drink)
{
  drink->num_drinks++;
  redrawText(drink);
}
void resetCounter(Drink* drink)
{
  drink->num_drinks=0;
  redrawText(drink);
}



#endif
