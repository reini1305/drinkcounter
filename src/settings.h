//
//  settings.h
//  drink_counter
//
//  Created by Christian Reinbacher on 28.07.14.
//
//

#ifndef drink_counter_settings_h
#define drink_counter_settings_h

// This is a custom defined key for saving our count field
#define SETTINGS_KEY 21

// You can define defaults for values in persistent storage
#define NUM_DRINKS_DEFAULT 0
#define NUM_DRINK_TYPES 7

typedef struct Settings
{
  int num_drinks[NUM_DRINK_TYPES];
  unsigned char drawing_order[NUM_DRINK_TYPES];
  struct tm last_drink_time;
  struct tm first_drink_time;
  int drink_meters;
  float drink_prices[NUM_DRINK_TYPES];
  bool estimate_bac;
  bool sex;
  int weight;
  bool unit;
  uint8_t beer_size;
  bool output;
}Settings;


#endif
