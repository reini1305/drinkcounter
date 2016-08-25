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

void createDrink(Drink* drink, Layer* parent_layer, uint32_t bitmap_id, int* num_drinks, int draw_slot, int grid_size_v);
void destroyDrink(Drink *drink);
void redrawText(Drink* drink);
void selectDrink(Drink* drink);
void deselectDrink(Drink* drink);
void increaseCounter(Drink* drink);
void resetCounter(Drink* drink);
void swapDrinks(Drink* drink1, Drink* drink2);
int getSumDrinks(Drink* drinks);



#endif
