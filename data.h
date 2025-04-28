#ifndef data_h
#define data_h

#include <stddef.h>
#include <time.h>

#define FACILITY_COUNT 5
#define MAX_MEAL_COUNT 100
#define MAX_ID_DIGITS 6 // 1 million
#define MAX_CAPACITY 1000

typedef enum {
  WIFI = 0,
  LIGHTNING = 1,
  PARKING = 2,
  SMOKING = 3,
  MUSIC = 4
} facility;

typedef struct {
  char *name;
  double price;
} Meal;

typedef struct {
  size_t id;
  char *name;
  Meal **meals;
  size_t meal_count;
} Menu;

typedef struct {
  size_t id;
  char *name;
  facility *facilities;
  size_t facility_count;
  int is_booked;
  size_t capacity;
  double price_per_hour;
} Room;

typedef struct {
  size_t id;
  size_t number_of_hours;

  Room *room;
  Menu *menu;

  struct tm *created_at;
} Reservation;

typedef struct {
  Menu **menus;
  size_t menu_count;
  Room **rooms;
  size_t room_count;
  Reservation **reservations;
  size_t reservation_count;
} App;

char *facility_name(facility facility);

facility *parse_facilities(const char *facility_str);

void print_facilities(facility *facilities, size_t n);

void print_room(Room room);

Room *create_room_from_file(App *app, int id, const char *name,
                            facility *facilities, size_t facility_cnt,
                            int capacity, int is_booked, double price_per_hour);

Room *create_room(App *app, const char *name, facility *facilities,
                  size_t facility_cnt, int capacity, double price_per_hour);

void print_menu(Menu menu);

Meal *create_meal(const char *name, double price);

Menu *create_menu_from_file(App *app, int id, const char *name, Meal **meals,
                            size_t meal_cnt);
Menu *create_menu(App *app, const char *name, Meal **meals, size_t meal_cnt);

char *tm_to_iso8601(const struct tm *timeptr);

int parse_iso8601(const char *input, struct tm *out);

void print_reservation(Reservation reservation);

Reservation *create_reservation_from_file(App *app, int id, Room *r, Menu *m,
                                          size_t number_of_hours,
                                          struct tm *created_at);

Reservation *create_reservation(App *app, Room *r, Menu *m,
                                size_t number_of_hours);

#endif
