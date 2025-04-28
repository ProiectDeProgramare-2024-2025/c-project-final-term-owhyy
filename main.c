/*
Utilizatorul aplicației își poate rezerva o sală într-un restaurant.
Aplicația permite afișarea tuturor sălilor disponibile, căutarea unei săli după
capacitate sau facilități incluse (exemplu: iluminat, WiFi, parcare), selectarea
meniului și rezervarea sălii și anularea rezervării. Pentru a efectua
rezervarea, utilizatorul trebuie să selecteze capacitatea sălii, să introducă
perioada și să selecteze facilitățile dorite. După efectuarea rezervarii, se pot
verifica detaliile rezervării (perioada, prețul total, facilitățile incluse,
etc.). Folosiți mecanismul de manipulare a fișierelor pentru a prelua/salva
datele din/în fișier. De asemenea, folosiți structuri de date pentru
informațiile citite din fișier.
*/

#include "data.h"
#include "file.h"
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  App *app = read_json();
  run(app);

  free(app->menus);
  for (int i = 0; i < app->room_count; i++) {
    free(app->rooms[i]->facilities);
  }
  free(app->rooms);
  free(app->reservations);
  free(app);
}
