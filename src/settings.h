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
#define SETTINGS_KEY 18

// You can define defaults for values in persistent storage
#define NUM_DRINKS_DEFAULT 0

typedef struct Settings
{
  int num_drinks[5];
  unsigned char drawing_order[5];
  struct tm last_drink_time;
  struct tm first_drink_time;
  int drink_meters;
  float drink_prices[5];
}Settings;


#endif
