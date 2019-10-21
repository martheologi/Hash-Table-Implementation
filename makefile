CC = gcc
DEBUG = -g
CFLAGS = $(DEBUG)
DEPS = BF.h HT.h SHT.h
OBJECTS = main_test.c HT.c SHT.c
LIBS = BF_64.a

%.o: %.c $(DEPS)
	$(CC) -c $< $(CFLAGS)

HT: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LIBS) $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o
