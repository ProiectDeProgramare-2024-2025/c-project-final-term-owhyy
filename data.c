#include "data.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *facility_name(facility facility) {
  char *facility_str = NULL;
  switch (facility) {
  case WIFI:
    facility_str = "wifi";
    break;
  case LIGHTNING:
    facility_str = "lightning";
    break;
  case PARKING:
    facility_str = "parking";
    break;
  case SMOKING:
    facility_str = "smoking";
    break;
  case MUSIC:
    facility_str = "music";
    break;
  }
  return facility_str;
}

void print_facilities(facility *facilities, size_t n) {
  printf("Facilities: ");
  int i;
  for (i = 0; i < n; i++) {
    printf(BLUE("%s "), facility_name(facilities[i]));
  }
  printf("\n");
}

void print_menu(Menu menu) {
  printf("%ld. " BLUE("%s") ":\n", menu.id, menu.name);
  for (int i = 0; i < menu.meal_count; i++)
    printf("%s -- " BLUE("%.2lf\n"), menu.meals[i]->name, menu.meals[i]->price);
}

Menu *create_menu_from_file(App *app, int id, const char *name, Meal **meals,
                            size_t meal_cnt) {
  Menu *m = malloc(sizeof(Menu));
  if (m == NULL) {
    perror("Failed memory allocation");
    return NULL;
  }

  m->id = id;
  m->name = strdup(name);
  m->meals = meals;
  m->meal_count = meal_cnt;

  if (app->menus)
    app->menus[app->menu_count++] = m;

  return m;
}

Menu *create_menu(App *app, const char *name, Meal **meals, size_t meal_cnt) {
  int id = 0;
  for (int i = 0; i < app->menu_count; i++)
    if (id < app->menus[i]->id)
      id = app->menus[i]->id + 1;

  if (id == 0)
    id++;
  return create_menu_from_file(app, id, name, meals, meal_cnt);
}

void print_room(Room room) {
  printf("%ld. " BLUE("%s\n"), room.id, room.name);
  print_facilities(room.facilities, room.facility_count);
  printf("Capacity: " BLUE("%ld\n"), room.capacity);
  printf("Room is" BLUE("%s booked\n"), room.is_booked ? "" : " not");
  printf("Price per hour: " BLUE("%.2lf\n"), room.price_per_hour);
}

facility *parse_facilities(const char *facility_str) {
  facility *f = malloc(sizeof(facility) * strlen(facility_str));
  for (int i = 0; i < strlen(facility_str); i++) {
    facility fy = facility_str[i] - '0';
    if (fy >= 0 && fy <= FACILITY_COUNT)
      f[i] = fy;
    else {
      fprintf(stderr, "Invalid facility digit %c\n", facility_str[i]);
      return NULL;
    }
  }
  return f;
}

Meal *create_meal(const char *name, double price) {
  Meal *m = malloc(sizeof(Meal));
  if (m == NULL) {
    perror("Failed memory allocation");
    return NULL;
  }
  m->name = strdup(name);
  m->price = price;
  return m;
}

Room *create_room_from_file(App *app, int id, const char *name,
                            facility *facilities, size_t facility_cnt,
                            int capacity, int is_booked,
                            double price_per_hour) {
  Room *r = malloc(sizeof(Room));
  if (r == NULL) {
    perror("Failed to allocate memory");
    return NULL;
  }

  r->id = id;
  r->name = strdup(name);
  r->capacity = capacity;
  r->facilities = facilities;
  r->facility_count = facility_cnt;
  r->price_per_hour = price_per_hour;
  r->is_booked = is_booked;

  if (app->rooms)
    app->rooms[app->room_count++] = r;
  return r;
}

Room *create_room(App *app, const char *name, facility *facilities,
                  size_t facility_cnt, int capacity, double price_per_hour) {
  int id = 0;
  for (int i = 0; i < app->room_count; i++)
    if (app->rooms[i]->id > id)
      id = app->rooms[i]->id + 1;

  if (id == 0)
    id++;
  
  return create_room_from_file(app, id, name, facilities, facility_cnt,
                               capacity, 0, price_per_hour);
}

char *tm_to_iso8601(const struct tm *timeptr) {
  char *buffer = malloc(25);
  if (buffer) {
    strftime(buffer, 25, "%Y-%m-%dT%H:%M:%S", timeptr);
  }
  return buffer;
}

void print_reservation(Reservation reservation) {
  printf("Reservation number " BBLUE("%ld\n"), reservation.id);
  printf("Room " BLUE("%s") " reserved for " BLUE("%ld") " hours\n",
         reservation.room->name, reservation.number_of_hours);

  Menu menu = *reservation.menu;
  printf("Menu " BLUE("%s") ":\n", menu.name);
  for (int i = 0; i < menu.meal_count; i++)
    printf("%s -- " BLUE("%.2lf\n"), menu.meals[i]->name, menu.meals[i]->price);

  printf("Reservation was created on " BLUE("%s\n"),
         tm_to_iso8601(reservation.created_at));
  printf(BBLACK("Total price: ") BBLUE("%.2lf\n"),
         reservation.room->price_per_hour * reservation.number_of_hours);
}

int parse_iso8601(const char *input, struct tm *out) {
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;

  if (sscanf(input, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &min,
             &sec) != 6) {
    return 0;
  }

  memset(out, 0, sizeof(struct tm));
  out->tm_year = year - 1900;
  out->tm_mon = month - 1;
  out->tm_mday = day;
  out->tm_hour = hour;
  out->tm_min = min;
  out->tm_sec = sec;

  return 1;
}

Reservation *create_reservation_from_file(App *app, int id, Room *r, Menu *m,
                                          size_t number_of_hours,
                                          struct tm *created_at) {

  Reservation *res = malloc(sizeof(Reservation));
  if (res == NULL) {
    perror("Failed to allocate memory");
    return NULL;
  }
  res->id = ++id;
  res->room = r;
  res->menu = m;
  res->number_of_hours = number_of_hours;
  res->created_at = created_at;

  if (app->reservations)
    app->reservations[app->reservation_count++] = res;

  return res;
}

Reservation *create_reservation(App *app, Room *r, Menu *m,
                                size_t number_of_hours) {
  int id = 0;
  for (int i = 0; i < app->reservation_count; i++)
    if (id < app->reservations[i]->id)
      id = app->reservations[i]->id + 1;

  if (id == 0)
    id++;  

  time_t raw_time;
  struct tm *time_info;
  time(&raw_time);
  time_info = gmtime(&raw_time);
  return create_reservation_from_file(app, id, r, m, number_of_hours,
                                      time_info);
}
