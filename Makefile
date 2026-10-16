CC ?= gcc
AR ?= ar

CPPFLAGS += -D_POSIX_C_SOURCE=200809L -Isrc
CSTD      = -std=c17

CDBGFLAGS ?= -O0 -g
CFLAGS    ?= -O2

ARFLAGS ?= rcs

WARN = -Wall -Wextra -Wpedantic -Werror

LIBNAME  := libjalloc.a
SOURCES  := 
OBJECTS  := $(patsubst src/%.c,build/%.o,$(SOURCES))
DEPFILES := $(OBJECTS:.o=.d)

FLAG_FILE := build/.compile_flags

.PHONY: dbg mrproper

$(LIBNAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

dbg: CFLAGS := $(CDBGFLAGS)
dbg: $(LIBNAME)

$(FLAG_FILE): FORCE
	@mkdir -p build
	@echo "$(CC) $(CPPFLAGS) $(CSTD) $(CFLAGS) $(WARN)" | cmp -s - $@ || \
		echo "$(CC) $(CPPFLAGS) $(CSTD) $(CFLAGS) $(WARN)" > $@

build/%.o: src/%.c $(FLAG_FILE)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CSTD) $(CFLAGS) $(WARN) -MMD -MP -c $< -o $@

-include $(DEPFILES)

mrproper:
	rm -rf build $(LIBNAME)

.PHONY: FORCE
