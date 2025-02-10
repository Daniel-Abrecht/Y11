
CFLAGS += -std=c17 -g
CFLAGS += -Iinclude -Ibin/include -Ibuild/include
CFLAGS += -Wall -Wextra -pedantic -Wno-missing-braces

CFLAGS += -D_GNU_SOURCE
CXXFLAGS += -D_GNU_SOURCE

LDLIBS += -ldpa-utils

SOURCES=$(shell find src/ -type f -not -iname ".*" -not -iname "*~")
HEADERS=$(shell find include/ -type f -not -iname ".*" -not -iname "*~")

SOURCES += build/src/protocol.c
HEADERS += bin/include/Y11/protocol.h
HEADERS += build/include/-Y11/protocol.h

GEN_PROTOCOL += bin/include/Y11/protocol.h
GEN_PROTOCOL += build/include/-Y11/protocol.h
GEN_PROTOCOL += build/src/protocol.c

OBJECTS=$(patsubst %.c,build/%.c.o,$(SOURCES))

PREFIX ?= /usr

all: bin/Y11

$(GEN_PROTOCOL): build/.gen_protocol

bin/Y11: $(OBJECTS)
	mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

build/%.c.o: %.c $(HEADERS)
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

build/.gen_protocol: include/Y11/protocol.msg.h script/gen_protocol.py
	script/gen_protocol.py
	touch $@

install:
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 bin/Y11 $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)$(PREFIX)/include/Y11/
	install -m 644 bin/include/Y11/*.h include/Y11/*.h $(DESTDIR)$(PREFIX)/include/Y11/

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/Y11
	rm -r $(DESTDIR)$(PREFIX)/include/Y11/

clean:
	rm -rf build bin
