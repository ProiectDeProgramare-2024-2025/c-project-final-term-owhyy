#include "menu.h"
#include "utils.h"
#include "data.h"
#include "file.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_string(size_t limit) {
  printf(">  ");
  errno = 0;
  char *buff = malloc(limit);
  if (!fgets(buff, limit, stdin)) {
    free(buff);
    errno = EIO;
    perror(RED("String reading error"));
    return NULL;
  }
  if (!strchr(buff, '\n')) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
      ; // Clear stdin
    free(buff);
    errno = EIO;
    fprintf(stderr, RED("Input length past limit\n"));
    return NULL;
  }

  size_t len = strlen(buff);
  if (buff[len - 1] == '\n') {
    buff[len - 1] = '\0';
  }

  return buff;
}

int read_int(size_t limit) {
  errno = 0;
  char *buff = read_string(limit);
  if (errno != 0)
    return -1;

  if (!buff) {
    return -1;
  }

  char *end;
  int choice = strtol(buff, &end, 10);

  if (errno) {
    fprintf(stderr, RED("Failed to convert '%s' to an integer\n"), buff);
    goto fail;
  }

  if (buff == end) {
    fprintf(stderr, RED("Expected an integer, got '%s'\n"), buff);
    goto fail;
  }

  while (*end == ' ' || *end == '\t' || *end == '\n') {
    end++;
  }

  if (*end != '\0') {
    goto fail;
  }

  free(buff);
  return choice;

fail:
  free(buff);
  return -1;
}

double read_double() {
  errno = 0;
  char *buff = read_string(12);
  if (errno != 0)
    return -1;

  if (!buff) {
    return -1;
  }

  char *end;
  double value = strtod(buff, &end);

  if (errno) {
    fprintf(stderr, RED("Failed to convert '%s' to a decimal value\n"), buff);
    goto fail;
  }

  if (buff == end) {
    fprintf(stderr, RED("Expected a decimal value, got '%s'\n"), buff);
    goto fail;
  }

  while (*end == ' ' || *end == '\t' || *end == '\n') {
    end++;
  }

  if (*end != '\0') {
    goto fail;
  }

  free(buff);
  return value;

fail:
  free(buff);
  return -1;
}

void read_char() { free(read_string(99999)); }

Menu *select_menu(App *app) {
  printf(BBLACK("Available menus:\n"));
  for (int i = 0; i < app->menu_count; i++)
    print_menu(*app->menus[i]);

  int menu_id = read_int(MAX_ID_DIGITS);
  if (menu_id == 0 || menu_id == -1 || errno)
    return NULL;

  Menu *menu = NULL;
  for (int i = 0; i < app->menu_count; i++)
    if (app->menus[i]->id == menu_id)
      menu = app->menus[i];

  if (menu == NULL) {
    fprintf(stderr, RED("Not a valid menu number\n"));
    return NULL;
  }
  return menu;
}

Room *select_room(App *app) {
  printf("\e[1;1H\e[2J");
  printf("Available rooms:\n");
  for (int i = 0; i < app->room_count; i++)
    print_room(*app->rooms[i]);
  int room_id = read_int(MAX_ID_DIGITS);

  if (errno != 0 || room_id == -1) {
    printf(RED("Try again\n"));    
    return NULL;
  }

  Room *room = NULL;

  for (int i = 0; i < app->room_count; i++)
    if (app->rooms[i]->id == room_id)
      room = app->rooms[i];

  if (room == NULL) {
    printf(RED("Not a valid room number\n"));
    return NULL;
  }

  return room;
}

void appointment_menu(App *app) {
  printf("\e[1;1H\e[2J");
  printf(BBLACK("Appointment menu\n")"1. Create an appointment\n2. See appointments\n0. "
         "Go back\n\n");
  int choice = read_int(MAX_ID_DIGITS);
  if (choice == 0)
    return;
  if (choice == -1) {
    fprintf(stderr, RED("Invalid option\nEnter anything to continue\n\n"));
    read_char();
    appointment_menu(app);
  }

  if (choice == 1) {
    add_appointment(app);
    write_json(app);
  }
  if (choice == 2)
    print_appointments(app);
}

void add_appointment(App *app) {
  printf("\e[1;1H\e[2J");
  printf(BBLACK("Create an appointment\n"));
  if (app->menu_count == 0) {
    printf(RED("Cannot create appointment. Please add a menu first"));
    printf("\nEnter anything to go back\n\n");
    read_char();
    return;
  }
  if (app->room_count == 0) {
    printf(RED("Cannot create appointment. Please add a room first"));
    printf("\nEnter anything to go back\n\n");
    read_char();
    return;
  }  
  Room *room = select_room(app);
  while (room == NULL) {
    fprintf(stderr, RED("Try again\n\n"));
    room = select_room(app);
  }
  printf("\e[1;1H\e[2J");
  printf("Enter the number of hours you want to book the room for\n");
  int hours_to_book = read_int(4);
  while (hours_to_book < 0 || errno) {
    fprintf(stderr, RED("Try again\n\n"));
    hours_to_book = read_int(4);
  }

  printf("\e[1;1H\e[2J");

  Menu *menu = select_menu(app);
  while (menu == NULL) {
    fprintf(stderr, RED("Try again\n\n"));
    menu = select_menu(app);
  }
  Reservation *r = create_reservation(app, room, menu, hours_to_book);
  printf("\e[1;1H\e[2J");
  printf(GREEN("Reservation created successfully!\n") "Details of the reservation "
         "below:\n");
  print_reservation(*r);
  printf("\nEnter anything to go back\n\n");
  read_char();
}

void print_appointments(App *app) {
  printf("\e[1;1H\e[2J");
  printf(BBLACK("Current Reservations\n"));
  for (int i = 0; i < app->reservation_count; i++) {
    print_reservation(*app->reservations[i]);
    printf("\n");
  }
  printf("Enter anything to go back to the menu\n\n");
  read_char();
  printf("\e[1;1H\e[2J");
}

void menu_menu(App *app) {
  printf("\e[1;1H\e[2J");
  printf(BBLACK("Menu menu\n")"1. Create a menu\n2. See menus\n0. Go back\n\n");
  int choice = read_int(MAX_ID_DIGITS);
  if (choice == 0)
    return;
  if (choice == -1) {
    printf("Invalid option\nEnter anything to continue\n\n");
    read_char();
    menu_menu(app);
  }

  if (choice == 1) {
    add_menu(app);
    write_json(app);
  }
  if (choice == 2)
    print_menus(app);
}

Meal *add_meal() {
  printf("Enter name for meal\n\n");
  char *name = read_string(30);
  while (strlen(name) == 0) {
    fprintf(stderr, "Meal name cannot be empty! Try again\n\n");
    name = read_string(100);
  }
  printf("Enter price for meal\n\n");
  double price = read_double();
  while (errno != 0 || price < 0) {
    fprintf(stderr, "Try again\n\n");
    price = read_double();
  }
  return create_meal(name, price);
}

void add_menu(App *app) {
  printf("\e[1;1H\e[2J");
  printf(BBLACK("Add a menu\n")"Enter name\n\n");
  const char *name = read_string(100);
  while (strlen(name) == 0) {
    fprintf(stderr, RED("Menu name cannot be empty! Try again\n\n"));
    name = read_string(100);
  }
  printf("\e[1;1H\e[2J");
  printf("Enter meals\n");
  size_t meal_count = 0;
  Meal **meals = malloc(sizeof(Meal *) * 20);
  do {
    meals[meal_count++] = add_meal();
    printf("Meal " GREEN("successfully")" added to menu\n1. Add another meal\n2. Finish "
           "menu\n\n");
    int choice = read_int(CHOICE_MAX_LENGTH);
    while (choice == -1 || errno != 0) {
      fprintf(stderr, RED("Try again\n"));
      choice = read_int(CHOICE_MAX_LENGTH);
    }
    if (choice == 1)
      continue;
    else if (choice == 2)
      break;
  } while (1);
  Menu *menu = create_menu(app, name, meals, meal_count);
  printf("Menu added " GREEN("successfully\n")"Press anything to go back\n\n");
  read_char();
}

void print_menus(App *app) {
  printf("\e[1;1H\e[2J");
  printf(BBLACK("Current Menus\n"));
  for (int i = 0; i < app->menu_count; i++) {
    print_menu(*app->menus[i]);
    printf("\n");
  }
  printf("Enter anything to go back to the menu\n\n");
  read_char();
  printf("\e[1;1H\e[2J");
}

void room_menu(App *app) {
  printf("\e[1;1H\e[2J");
  printf(BBLACK("Room menu\n")"1. Add a room\n2. See rooms\n0. Go back\n\n");
  int choice = read_int(MAX_ID_DIGITS);
  if (choice == 0)
    return;
  if (choice == -1) {
    fprintf(stderr, RED("Invalid option\nEnter anything to continue\n\n"));
    read_char();
    menu_menu(app);
  }

  if (choice == 1) {
    add_room(app);
    write_json(app);
  }
  if (choice == 2)
    print_rooms(app);
}

void add_room(App *app) {
  printf("\e[1;1H\e[2J");
  printf(BBLACK("Add a room\n")"Enter name\n\n");
  const char *name = read_string(100);
  while (strlen(name) == 0) {
    fprintf(stderr, RED("Room name cannot be empty! Try again\n\n"));
    name = read_string(100);
  }
  printf("\e[1;1H\e[2J");
  printf("Enter facilities\n");
  for (int i = 0; i < FACILITY_COUNT; i++) {
    facility f = (facility)i;
    printf("%d. %s\n", f, facility_name(f));
  }
  printf("\n");
  const char *facility_str = read_string(FACILITY_COUNT);
  while (errno != 0) {
    fprintf(stderr, RED("Try again\n\n"));
    facility_str = read_string(FACILITY_COUNT);
  }

  facility *facilities = parse_facilities(facility_str);
  while (errno != 0 && strlen(facility_str) != 0 && facilities == NULL) {
    fprintf(stderr, RED("Try again\n\n"));
    facility_str = read_string(FACILITY_COUNT);
    facilities = parse_facilities(facility_str);
  }

  printf("\e[1;1H\e[2J");
  printf("Enter room capacity\n\n");
  int capacity = read_int(6);
  while (errno != 0 || capacity == -1) {
    fprintf(stderr, RED("Try again\n\n"));
    capacity = read_int(6);
  }

  printf("\e[1;1H\e[2J");
  printf("Enter price per hour\n\n");
  double price_per_hour = read_double();
  while (errno != 0 || price_per_hour == -1) {
    fprintf(stderr, RED("Try again\n\n"));
    price_per_hour = read_double();
  }
  Room *r = create_room(app, name, facilities, strlen(facility_str), capacity,
                        price_per_hour);

  printf("Room " BLUE("%s") GREEN(" successfully ") "added\nPress anything to go back\n\n", r->name);
  read_char();
}

void print_rooms(App *app) {
  printf("\e[1;1H\e[2J");
  printf(BBLACK("Current Rooms\n"));
  for (int i = 0; i < app->room_count; i++) {
    print_room(*app->rooms[i]);
    printf("\n");
  }
  printf("Enter anything to go back to the menu\n\n");
  read_char();
  printf("\e[1;1H\e[2J");
}

void run(App *app) {
  while (1) {
    printf(BBLACK("Main menu\n")"1. Appointments\n2. Rooms\n"
           "3. Menus\n0. Quit\n\n");

    int choice = read_int(CHOICE_MAX_LENGTH);
    switch (choice) {
    case 1:
      appointment_menu(app);
      break;
    case 2:
      room_menu(app);
      break;
    case 3:
      menu_menu(app);
      break;
    case 0:
      return;
    default:
      printf("\e[1;1H\e[2J");
      fprintf(stderr, RED("Invalid option\n"));
      break;
    }
    printf("\e[1;1H\e[2J");
  }
}
