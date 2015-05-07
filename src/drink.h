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
  //InverterLayer* inverter_layer;
  bool is_selected;
  TextLayer* text_layer;
  int *num_drinks;
  //unsigned char storage_slot;
  unsigned char draw_slot;
  unsigned char width;
  char text[3];
  GFont font;
}Drink;

void loadDrink(Drink *drink)
{
  //drink->num_drinks = persist_exists(drink->storage_slot) ? persist_read_int(drink->storage_slot) : 0;
}

void saveDrink(Drink *drink)
{
  //persist_write_int(drink->storage_slot, drink->num_drinks);
}

void createDrink(Drink* drink, Layer* parent_layer, uint32_t bitmap_id, int* num_drinks, int draw_slot, int grid_size_v)
{
  int text_y = 50;
  int text_height = 100;
  drink->text_layer = text_layer_create(GRect(0, text_y, grid_size_v, text_height));
  drink->font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_30));
  text_layer_set_font(drink->text_layer, drink->font);
  //text_layer_set_font(drink->text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_background_color(drink->text_layer, GColorClear);
  text_layer_set_text_alignment(drink->text_layer,GTextAlignmentCenter);
  layer_add_child(parent_layer, text_layer_get_layer(drink->text_layer));
  
  drink->bitmap = gbitmap_create_with_resource(bitmap_id);
  drink->bitmap_layer = bitmap_layer_create(GRect(0, 0, grid_size_v, 40));
  bitmap_layer_set_alignment(drink->bitmap_layer,GAlignCenter);
  bitmap_layer_set_bitmap(drink->bitmap_layer,drink->bitmap);
  layer_add_child(parent_layer,bitmap_layer_get_layer(drink->bitmap_layer));
  
  //drink->inverter_layer = inverter_layer_create(GRect(0, text_y, grid_size_v, text_height));
  //layer_set_hidden(inverter_layer_get_layer(drink->inverter_layer),true);
  //layer_add_child(parent_layer,inverter_layer_get_layer(drink->inverter_layer));
  //drink->storage_slot = storage_id;
  drink->is_selected = false;
  drink->draw_slot=draw_slot;
  drink->width=grid_size_v;
  drink->num_drinks=num_drinks;
  //APP_LOG(APP_LOG_LEVEL_DEBUG,"Drink %d,%d\n",draw_slot,draw_slot*grid_size_v);
  loadDrink(drink);
}

void destroyDrink(Drink *drink)
{
  saveDrink(drink);
  fonts_unload_custom_font(drink->font);
  text_layer_destroy(drink->text_layer);
  //inverter_layer_destroy(drink->inverter_layer);
  bitmap_layer_destroy(drink->bitmap_layer);
  gbitmap_destroy(drink->bitmap);
}

void redrawText(Drink* drink)
{
  int text_y = 50;
  int text_height = 100;
  
  if(drink->is_selected)
  {
    text_layer_set_text_color(drink->text_layer,GColorWhite);
    text_layer_set_background_color(drink->text_layer,GColorBlack);
  }
  else
  {
    text_layer_set_text_color(drink->text_layer,GColorBlack);
    text_layer_set_background_color(drink->text_layer,GColorWhite);
  }
  snprintf(drink->text, sizeof(drink->text), "%u", *drink->num_drinks);
  text_layer_set_text(drink->text_layer, drink->text);
  
  // Move layers to the correct position
  layer_set_frame(text_layer_get_layer(drink->text_layer),GRect(drink->draw_slot*drink->width, text_y, drink->width, text_height));
  layer_set_frame(bitmap_layer_get_layer(drink->bitmap_layer),GRect(drink->draw_slot*drink->width, 0, drink->width, 40));
  //layer_set_frame(inverter_layer_get_layer(drink->inverter_layer),GRect(drink->draw_slot*drink->width, text_y, drink->width, text_height));
}

void selectDrink(Drink* drink)
{
  //layer_set_hidden(inverter_layer_get_layer(drink->inverter_layer),false);
  drink->is_selected = true;
  redrawText(drink);
}

void deselectDrink(Drink* drink)
{
  //layer_set_hidden(inverter_layer_get_layer(drink->inverter_layer),true);
  drink->is_selected = false;
  redrawText(drink);
}

void increaseCounter(Drink* drink)
{
  *(drink->num_drinks)=*(drink->num_drinks)+1;
  redrawText(drink);
}
void resetCounter(Drink* drink)
{
  *drink->num_drinks=0;
  redrawText(drink);
}
void swapDrinks(Drink* drink1, Drink* drink2)
{
  unsigned char temp = drink1->draw_slot;
  drink1->draw_slot=drink2->draw_slot;
  drink2->draw_slot=temp;
  redrawText(drink1);
  redrawText(drink2);
}


#endif
