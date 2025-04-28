#ifndef menu_h
#define menu_h

#define CHOICE_MAX_LENGTH 3 // 2 + \0

#include "data.h"


char *read_string(size_t limit);

int read_int(size_t limit);

void run(App *app);

Menu *select_menu(App *app);

Room *select_room(App *app);

void appointment_menu(App *app);

void add_appointment(App *app);

void print_appointments(App *app);

void menu_menu(App *app);

void add_menu(App *app);

void print_menus(App *app);

void room_menu(App *app);

void add_room(App *app);

void print_rooms(App *app);

#endif
