#ifndef file__h
#define file__h

#include "cjson.h"
#include "data.h"

#define JSON_FILENAME "app.json"

App *read_json();
void write_json(const App *app);

#endif
