CC       ?= cc
AR       ?= ar
ARFLAGS  = rcs

BUILD_DIR      ?= build
STATIC_LIBRARY ?= libjalloc.a
SHARED_LIBRARY ?= libjalloc.so
TEST_BINARY    ?= $(BUILD_DIR)/test/jalloc-test
TEST_LIBRARY   ?= $(BUILD_DIR)/test/libjalloc.a
DOXYGEN        ?= doxygen
DOCS_DIR       ?= docs

CFLAGS   ?= -O2
CDBGFLAGS ?= -O0 -g
CSTD     := -std=c17
WARNINGS ?= -Wall -Wextra -Wpedantic -Werror

PROJECT_CPPFLAGS := -D_POSIX_C_SOURCE=200809L -Iinclude -Isrc
TEST_CPPFLAGS    := -DJALLOC_UNITTESTING -Itest -Itest/third-party
BASE_CPPFLAGS    := $(CPPFLAGS) $(PROJECT_CPPFLAGS)
BASE_CFLAGS      = $(CFLAGS) $(CSTD) $(WARNINGS)

LIB_SOURCES := $(wildcard src/*.c src/*/*.c)

TEST_SOURCES := $(sort $(wildcard test/*.c test/*/*.c))

STATIC_OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/static/%.o,$(LIB_SOURCES))
SHARED_OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/shared/%.o,$(LIB_SOURCES))
TEST_OBJECTS   := $(patsubst test/%.c,$(BUILD_DIR)/test/%.o,$(TEST_SOURCES))
TEST_LIBRARY_OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/test-lib/%.o,$(LIB_SOURCES))
DEPFILES       := $(STATIC_OBJECTS:.o=.d) $(SHARED_OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d) $(TEST_LIBRARY_OBJECTS:.o=.d)

STATIC_COMPILE_FLAGS := $(BUILD_DIR)/.static-compile-flags
SHARED_COMPILE_FLAGS := $(BUILD_DIR)/.shared-compile-flags
TEST_COMPILE_FLAGS   := $(BUILD_DIR)/.test-compile-flags
TEST_LIBRARY_COMPILE_FLAGS := $(BUILD_DIR)/.test-library-compile-flags
STATIC_ARCHIVE_FLAGS := $(BUILD_DIR)/.static-archive-flags
LINK_FLAGS           := $(BUILD_DIR)/.link-flags
SHARED_EXPORTS       := $(BUILD_DIR)/jalloc.map

.DEFAULT_GOAL := $(STATIC_LIBRARY)
.DELETE_ON_ERROR:
.PHONY: dbg shared test docs mrproper FORCE

$(STATIC_LIBRARY): $(STATIC_OBJECTS) $(STATIC_ARCHIVE_FLAGS)
	@mkdir -p "$(@D)"
	@rm -f "$@"
	$(AR) $(ARFLAGS) $@ $(STATIC_OBJECTS)

dbg: CFLAGS := $(CDBGFLAGS)
dbg: $(STATIC_LIBRARY)

shared: $(SHARED_LIBRARY)

$(SHARED_LIBRARY): $(SHARED_OBJECTS) $(SHARED_EXPORTS) $(LINK_FLAGS)
	@mkdir -p "$(@D)"
	$(CC) -shared $(LDFLAGS) -Wl,--version-script=$(SHARED_EXPORTS) -o $@ $(SHARED_OBJECTS) $(LDLIBS)

test: $(TEST_BINARY)
	$(TEST_BINARY)

docs:
	@printf '%s\n' \
		'PROJECT_NAME = jalloc' \
		'OUTPUT_DIRECTORY = "$(DOCS_DIR)"' \
		'INPUT = include src' \
		'RECURSIVE = YES' \
		'FILE_PATTERNS = *.h *.c' \
		'EXTRACT_ALL = YES' \
		'GENERATE_HTML = YES' \
		'GENERATE_LATEX = NO' \
		'QUIET = YES' | $(DOXYGEN) -

$(TEST_BINARY): $(TEST_OBJECTS) $(TEST_LIBRARY) $(LINK_FLAGS)
	@mkdir -p "$(@D)"
	$(CC) $(LDFLAGS) -o $@ $(TEST_OBJECTS) $(TEST_LIBRARY) $(LDLIBS) -lm

$(TEST_LIBRARY): $(TEST_LIBRARY_OBJECTS) $(STATIC_ARCHIVE_FLAGS)
	@mkdir -p "$(@D)"
	@rm -f "$@"
	$(AR) $(ARFLAGS) $@ $(TEST_LIBRARY_OBJECTS)

$(BUILD_DIR)/static/%.o: src/%.c $(STATIC_COMPILE_FLAGS)
	@mkdir -p "$(@D)"
	$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/shared/%.o: src/%.c $(SHARED_COMPILE_FLAGS)
	@mkdir -p "$(@D)"
	$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) -fPIC -MMD -MP -c $< -o $@

$(BUILD_DIR)/test/%.o: test/%.c $(TEST_COMPILE_FLAGS)
	@mkdir -p "$(@D)"
	$(CC) $(BASE_CPPFLAGS) $(TEST_CPPFLAGS) $(BASE_CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/test-lib/%.o: src/%.c $(TEST_LIBRARY_COMPILE_FLAGS)
	@mkdir -p "$(@D)"
	$(CC) $(BASE_CPPFLAGS) -DJALLOC_UNITTESTING $(BASE_CFLAGS) -MMD -MP -c $< -o $@

$(STATIC_COMPILE_FLAGS): BUILD_FLAGS = $(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS)
$(SHARED_COMPILE_FLAGS): BUILD_FLAGS = $(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) -fPIC
$(TEST_COMPILE_FLAGS): BUILD_FLAGS = $(CC) $(BASE_CPPFLAGS) $(TEST_CPPFLAGS) $(BASE_CFLAGS)
$(TEST_LIBRARY_COMPILE_FLAGS): BUILD_FLAGS = $(CC) $(BASE_CPPFLAGS) -DJALLOC_UNITTESTING $(BASE_CFLAGS)
$(STATIC_ARCHIVE_FLAGS): BUILD_FLAGS = $(AR) $(ARFLAGS)
$(LINK_FLAGS): BUILD_FLAGS = $(CC) $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/.%: FORCE
	@mkdir -p "$(@D)"
	@printf '%s\n' '$(BUILD_FLAGS)' | cmp -s - "$@" || printf '%s\n' '$(BUILD_FLAGS)' > "$@"

$(SHARED_EXPORTS): Makefile
	@mkdir -p "$(@D)"
	@printf '%s\n' '{' '  global:' '    jalloc;' '    jcalloc;' '    jrealloc;' '    jfree;' '  local:' '    *;' '};' > "$@"

mrproper:
	rm -rf "$(BUILD_DIR)" "$(STATIC_LIBRARY)" "$(SHARED_LIBRARY)" "$(TEST_BINARY)" "$(TEST_LIBRARY)" "$(DOCS_DIR)"

-include $(DEPFILES)

FORCE:
