#include "file.h"
#include "cjson.h"
#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

size_t _count_elements(const cJSON *elements) {
  size_t count = 0;
  const cJSON *elem = NULL;
  cJSON_ArrayForEach(elem, elements) { count++; }
  return count;
}

int read_rooms(App *app, const cJSON *rooms) {
  const cJSON *room = NULL;

  cJSON_ArrayForEach(room, rooms) {
    const cJSON *id = cJSON_GetObjectItemCaseSensitive(room, "id");
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(room, "name");
    const char *facility_str =
        cJSON_GetObjectItemCaseSensitive(room, "facilities")->valuestring;
    const cJSON *is_booked =
        cJSON_GetObjectItemCaseSensitive(room, "is_booked");
    const cJSON *capacity = cJSON_GetObjectItemCaseSensitive(room, "capacity");
    const cJSON *price_per_hour =
        cJSON_GetObjectItemCaseSensitive(room, "price_per_hour");

    facility *facilities = NULL;
    if (strlen(facility_str)) {
      facilities = parse_facilities(facility_str);
      if (facilities == NULL) {
        perror("Invalid facilities for room");
        return -1;
      }
    }
    Room *r = create_room_from_file(
        app, id->valueint, name->valuestring, facilities, strlen(facility_str),
        capacity->valueint, is_booked->valueint, price_per_hour->valuedouble);
    if (r == NULL) {
      fprintf(stderr, "Failed to read room %s from file\n", name->valuestring);
      return -1;
    }
  }
  return 1;
}

int read_menus(App *app, const cJSON *menus) {
  const cJSON *menu = NULL;

  cJSON_ArrayForEach(menu, menus) {
    const cJSON *id = cJSON_GetObjectItemCaseSensitive(menu, "id");
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(menu, "name");
    const cJSON *meals = cJSON_GetObjectItemCaseSensitive(menu, "meals");

    size_t meals_cnt = 0;
    Meal **menu_meals = NULL;
    if (meals) {
      const cJSON *meal = NULL;
      size_t menu_count = _count_elements(meals);
      if (menu_count) {
        menu_meals = malloc(sizeof(Meal) * menu_count);

        cJSON_ArrayForEach(meal, meals) {
          const cJSON *meal_name =
              cJSON_GetObjectItemCaseSensitive(meal, "name");
          const cJSON *price = cJSON_GetObjectItemCaseSensitive(meal, "price");
          menu_meals[meals_cnt] =
              create_meal(meal_name->valuestring, price->valuedouble);
          meals_cnt++;
        }
      }
    }

    Menu *m = create_menu_from_file(app, id->valueint, name->valuestring,
                                    menu_meals, meals_cnt);
    if (m == NULL) {
      fprintf(stderr, "Failed reading menu %s from file\n", name->valuestring);
      return -1;
    }
  }
  return 1;
}

int read_reservations(App *app, const cJSON *reservations) {
  const cJSON *reservation = NULL;

  cJSON_ArrayForEach(reservation, reservations) {
    const cJSON *id = cJSON_GetObjectItemCaseSensitive(reservation, "id");
    const cJSON *number_of_hours =
        cJSON_GetObjectItemCaseSensitive(reservation, "number_of_hours");
    const cJSON *room_id =
        cJSON_GetObjectItemCaseSensitive(reservation, "room");
    const cJSON *menu_id =
        cJSON_GetObjectItemCaseSensitive(reservation, "menu");
    const cJSON *created_at =
        cJSON_GetObjectItemCaseSensitive(reservation, "created_at");

    Room *r = NULL;
    for (int i = 0; i < app->room_count; i++) {
      if (app->rooms && app->rooms[i]->id == room_id->valueint)
        r = app->rooms[i];
    }

    Menu *m = NULL;
    for (int i = 0; i < app->menu_count; i++) {
      if (app->menus && app->menus[i]->id == menu_id->valueint)
        m = app->menus[i];
    }

    struct tm *created_at_tm = malloc(sizeof(struct tm));
    if (parse_iso8601(created_at->valuestring, created_at_tm) == 0) {
      fprintf(stderr, "Invalid datetime format %s.\n", created_at->valuestring);
      return -1;
    }

    Reservation *rn = create_reservation_from_file(
        app, id->valueint, r, m, number_of_hours->valueint, created_at_tm);
    if (rn == NULL) {
      fprintf(stderr, "Failed to read reservation %d from file\n",
              id->valueint);
      return -1;
    }
  }
  return 1;
}

App *create_app() {
  App *app = malloc(sizeof(App));
  app->rooms = malloc(sizeof(Room*) * 50);
  app->room_count = 0;
  app->menus = malloc(sizeof(Menu*) * 50);
  app->menu_count = 0;
  app->reservations = malloc(sizeof(Reservation*) * 50);
  app->reservation_count = 0;
  return app;
}

App *read_app(const char *json) {
  App *app = create_app();
  if (app == NULL)
    return NULL;

  cJSON *app_json = cJSON_Parse(json);
  if (app_json == NULL) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
      fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    goto end;
  }
  app_json = cJSON_GetObjectItemCaseSensitive(app_json, "app");

  const cJSON *rooms = cJSON_GetObjectItemCaseSensitive(app_json, "rooms");
  if (rooms) {
    size_t room_count = _count_elements(rooms);
    if (room_count)
      app->rooms = malloc(sizeof(Room) * room_count);
    if (app->rooms == NULL) {
      printf("Failed to allocate memory");
      free(app);
      return NULL;
    }
    app->room_count = 0;
    read_rooms(app, rooms);
  }

  const cJSON *menus = cJSON_GetObjectItemCaseSensitive(app_json, "menus");
  if (menus) {
    size_t menu_count = _count_elements(menus);
    if (menu_count)
      app->menus = malloc(sizeof(Menu *) * _count_elements(menus));
    read_menus(app, menus);
  }

  const cJSON *reservations =
      cJSON_GetObjectItemCaseSensitive(app_json, "reservations");
  if (reservations) {
    size_t reservation_count = _count_elements(reservations);
    if (reservation_count)
      app->reservations = malloc(sizeof(Reservation) * reservation_count);
    app->reservation_count = 0;
    read_reservations(app, reservations);
  }
end:
  cJSON_Delete(app_json);
  return app;
}

char *read_file() {
  FILE *file = fopen(JSON_FILENAME, "rb");
  if (!file) {
    perror("Failed to open file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  if (size == 0) {
    return NULL;
  }
  rewind(file);

  char *buffer = malloc(size + 1);
  if (!buffer) {
    perror("Failed to allocate memory");
    fclose(file);
    free(buffer);
    return NULL;
  }

  fread(buffer, 1, size, file);
  buffer[size] = '\0';

  fclose(file);
  return buffer;
}

App *read_json() {
  App *app = create_app();
  char *file = read_file();
  if (file == NULL) {
    return app;
  }
  app = read_app(file);
  free(file);
  return app;
}

void write_string_to_file(const char *text) {
  FILE *file = fopen(JSON_FILENAME, "w");

  if (file == NULL) {
    perror("Error opening file");
    return;
  }

  fputs(text, file);
  fclose(file);
}

cJSON *app_to_json(const App *app) {
  cJSON *root = cJSON_CreateObject();
  cJSON *app_obj = cJSON_AddObjectToObject(root, "app");

  cJSON *reservations_arr = cJSON_AddArrayToObject(app_obj, "reservations");
  for (size_t i = 0; i < app->reservation_count; ++i) {
    Reservation *res = app->reservations[i];
    cJSON *res_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(res_obj, "id", res->id);
    cJSON_AddNumberToObject(res_obj, "number_of_hours", res->number_of_hours);
    cJSON_AddNumberToObject(res_obj, "room", res->room->id);
    cJSON_AddNumberToObject(res_obj, "menu", res->menu->id);
    char *created_str = tm_to_iso8601(res->created_at);
    cJSON_AddStringToObject(res_obj, "created_at", created_str);
    free(created_str);
    cJSON_AddItemToArray(reservations_arr, res_obj);
  }

  cJSON *rooms_arr = cJSON_AddArrayToObject(app_obj, "rooms");
  for (size_t i = 0; i < app->room_count; ++i) {
    Room *room = app->rooms[i];
    cJSON *room_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(room_obj, "id", room->id);
    cJSON_AddStringToObject(room_obj, "name", room->name);

    char facilities_str[16] = {0};
    for (size_t j = 0; j < room->facility_count; ++j) {
      char digit[2];
      snprintf(digit, 2, "%d", room->facilities[j]);
      strcat(facilities_str, digit);
    }
    cJSON_AddStringToObject(room_obj, "facilities", facilities_str);

    cJSON_AddBoolToObject(room_obj, "is_booked", room->is_booked);
    cJSON_AddNumberToObject(room_obj, "capacity", room->capacity);
    cJSON_AddNumberToObject(room_obj, "price_per_hour", room->price_per_hour);
    cJSON_AddItemToArray(rooms_arr, room_obj);
  }

  cJSON *menus_arr = cJSON_AddArrayToObject(app_obj, "menus");
  for (size_t i = 0; i < app->menu_count; ++i) {
    Menu *menu = app->menus[i];
    cJSON *menu_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(menu_obj, "id", menu->id);
    cJSON_AddStringToObject(menu_obj, "name", menu->name);

    cJSON *meals_arr = cJSON_AddArrayToObject(menu_obj, "meals");
    for (size_t j = 0; j < menu->meal_count; ++j) {
      Meal *meal = menu->meals[j];
      cJSON *meal_obj = cJSON_CreateObject();
      cJSON_AddStringToObject(meal_obj, "name", meal->name);
      cJSON_AddNumberToObject(meal_obj, "price", meal->price);
      cJSON_AddItemToArray(meals_arr, meal_obj);
    }

    cJSON_AddItemToArray(menus_arr, menu_obj);
  }

  return root;
}

void write_json(const App *app) {
  const char *json = cJSON_Print(app_to_json(app));
  printf("%s", json);
  write_string_to_file(json);
}
