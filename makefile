
CFLAGS += -std=c17 -g
CFLAGS += -Iinclude
CFLAGS += -Wall -Wextra -pedantic
CFLAGS += -D_DEFAULT_SOURCE

SOURCES=$(shell find src/ -type f)
HEADERS=$(shell find include/ -type f)
OBJECTS=$(patsubst src/%.c,build/%.c.o,$(SOURCES))

all: bin/Y11

bin/Y11: $(OBJECTS)
	mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

build/%.c.o: src/%.c $(HEADERS)
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@
