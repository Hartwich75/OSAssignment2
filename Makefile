CC = gcc

CCWARNINGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable
CCOPTS     = -std=c11 -g -O0

CFLAGS = $(CCWARNINGS) $(CCOPTS)

TEST_SOURCES := test_mm.c mm.c memory_setup.c
TEST_OBJECTS := $(TEST_SOURCES:.c=.o)

CHECK_SOURCES := check_mm.c mm.c memory_setup.c
CHECK_OBJECTS := $(CHECK_SOURCES:.c=.o)

APP_SOURCES := main.c io.c mm.c memory_setup.c
APP_OBJECTS := $(APP_SOURCES:.c=.o)

TEST_EXECUTABLE = mm_test
CHECK_EXECUTABLE = malloc_check
APP_EXECUTABLE  = cmd_int

.PHONY: all clean

all: $(TEST_EXECUTABLE) $(CHECK_EXECUTABLE) $(APP_EXECUTABLE)

%.o: %.c mm.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_EXECUTABLE): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -o $@ 

$(CHECK_EXECUTABLE): $(CHECK_OBJECTS)
	$(CC) $(CFLAGS) $(CHECK_OBJECTS) -o $@ -lcheck -lsubunit -lm

$(APP_EXECUTABLE): $(APP_OBJECTS)
	$(CC) $(CFLAGS) $(APP_OBJECTS) -o $@

clean:
	rm -rf *o *~ $(TEST_EXECUTABLE) $(CHECK_EXECUTABLE) $(APP_EXECUTABLE)

