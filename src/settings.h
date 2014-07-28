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
#define NUM_BEERS_PKEY 1
#define NUM_WINE_PKEY 2
#define NUM_COCKTAILS_PKEY 3
#define NUM_SHOTS_PKEY 5
#define LAST_DRINK_TIME 4
#define FIRST_DRINK_TIME 12 // 6 to 11 are used for config!
#define NUM_CIGARETTES_PKEY 13
#define DRAWING_ORDER_PKEY 14
#define LAT_KEY 15
#define LON_KEY 16
#define SETTINGS_KEY 17

// You can define defaults for values in persistent storage
#define NUM_DRINKS_DEFAULT 0

typedef struct Settings
{
  int num_drinks[5];
  unsigned char drawing_order[5];
  struct tm last_drink_time;
  struct tm first_drink_time;
  int drink_meters;
}Settings;


#endif
