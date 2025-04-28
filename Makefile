CC=gcc
CFLAGS=-I.
DEPS = data.h menu.h file.h cjson.h utils.h
OBJ = main.o data.o menu.o file.o cjson.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

event-room-reservation: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

all: event-room-reservation

clean:
	rm -f *.o event-room-reservation
