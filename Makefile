# i don't want to waste my time writing makefile, so it was written by GPT-6-Luna.
# TODO: rewrite it by myself (someday)

# Select the host API and toolchain with PLATFORM=posix or PLATFORM=windows.
# The win* targets below are convenient Windows/MinGW-w64 entry points.
PLATFORM ?= posix

BUILD_ROOT ?= build
WIN_BUILD_DIR ?= $(BUILD_ROOT)/windows

POSIX_CC ?= cc
POSIX_AR ?= ar
WIN_CC ?= x86_64-w64-mingw32-gcc
WIN_AR ?= ar

ifeq ($(PLATFORM),posix)
CC = $(POSIX_CC)
AR = $(POSIX_AR)

BUILD_DIR ?= $(BUILD_ROOT)
STATIC_LIBRARY ?= libjalloc.a
SHARED_LIBRARY ?= libjalloc.so
TEST_BINARY ?= $(BUILD_DIR)/test/jalloc-test
TEST_LIBRARY ?= $(BUILD_DIR)/test/libjalloc.a

PLATFORM_CPPFLAGS = -D_POSIX_C_SOURCE=200809L
PICFLAGS = -fPIC
PLATFORM_CFLAGS =
SHARED_EXPORTS ?= $(BUILD_DIR)/jalloc.map
SHARED_PLATFORM_FLAGS = -Wl,--version-script=$(SHARED_EXPORTS)
SHARED_LINKER_INPUTS =
else ifeq ($(PLATFORM),windows)
CC = $(WIN_CC)
AR = $(WIN_AR)

BUILD_DIR ?= $(WIN_BUILD_DIR)
STATIC_LIBRARY ?= $(BUILD_DIR)/libjalloc.a
SHARED_LIBRARY ?= $(BUILD_DIR)/jalloc.dll
SHARED_IMPORT_LIBRARY ?= $(BUILD_DIR)/libjalloc.dll.a
TEST_BINARY ?= $(BUILD_DIR)/test/jalloc-test.exe
TEST_LIBRARY ?= $(BUILD_DIR)/test/libjalloc.a

# Do not define _POSIX_C_SOURCE here: src/os/mem.c uses it to select its
# POSIX implementation ahead of the _WIN32 implementation.
PLATFORM_CPPFLAGS =
PICFLAGS =
# josmemreturn() deliberately ignores sz on Windows; keep the diagnostic but
# don't let this intentional unused parameter defeat the otherwise strict build.
PLATFORM_CFLAGS = -Wno-error=unused-parameter
SHARED_EXPORTS ?= $(BUILD_DIR)/jalloc.def
SHARED_PLATFORM_FLAGS = -Wl,--out-implib,$(SHARED_IMPORT_LIBRARY)
SHARED_LINKER_INPUTS = $(SHARED_EXPORTS)
else
$(error Unsupported PLATFORM '$(PLATFORM)'; use PLATFORM=posix or PLATFORM=windows)
endif

ARFLAGS = rcs
CFLAGS ?= -O2
CDBGFLAGS ?= -O0 -g
CSTD ?= -std=c17
WARNINGS ?= -Wall -Wextra -Wpedantic -Werror

PROJECT_CPPFLAGS = -Iinclude -Isrc $(PLATFORM_CPPFLAGS)
TEST_CPPFLAGS = -DJALLOC_UNITTESTING -Itest -Itest/third-party
BASE_CPPFLAGS = $(CPPFLAGS) $(PROJECT_CPPFLAGS)
BASE_CFLAGS = $(CFLAGS) $(CSTD) $(WARNINGS) $(PLATFORM_CFLAGS)

DOXYGEN ?= doxygen
DOCS_DIR ?= docs

LIB_SOURCES = $(sort $(wildcard src/*.c src/*/*.c))
TEST_SOURCES = $(sort $(wildcard test/*.c test/*/*.c))

STATIC_OBJECTS = $(patsubst src/%.c,$(BUILD_DIR)/static/%.o,$(LIB_SOURCES))
SHARED_OBJECTS = $(patsubst src/%.c,$(BUILD_DIR)/shared/%.o,$(LIB_SOURCES))
TEST_OBJECTS = $(patsubst test/%.c,$(BUILD_DIR)/test/%.o,$(TEST_SOURCES))
TEST_LIBRARY_OBJECTS = $(patsubst src/%.c,$(BUILD_DIR)/test-lib/%.o,$(LIB_SOURCES))

STATIC_FLAGS_FILE = $(BUILD_DIR)/.static-compile-flags
SHARED_FLAGS_FILE = $(BUILD_DIR)/.shared-compile-flags
TEST_FLAGS_FILE = $(BUILD_DIR)/.test-compile-flags
TEST_LIBRARY_FLAGS_FILE = $(BUILD_DIR)/.test-library-compile-flags
STATIC_ARCHIVE_FLAGS_FILE = $(BUILD_DIR)/.static-archive-flags
TEST_ARCHIVE_FLAGS_FILE = $(BUILD_DIR)/.test-archive-flags
LINK_FLAGS_FILE = $(BUILD_DIR)/.link-flags

DEPFILES = $(STATIC_OBJECTS:.o=.d) $(SHARED_OBJECTS:.o=.d) \
	$(TEST_OBJECTS:.o=.d) $(TEST_LIBRARY_OBJECTS:.o=.d)

.DEFAULT_GOAL := all
.DELETE_ON_ERROR:
.PHONY: all dbg shared test docs mrproper win win-shared win-test FORCE

all: $(STATIC_LIBRARY)

# Convenience targets for a MinGW-w64 Windows build. Equivalent explicit
# invocations are: make PLATFORM=windows, ... shared, or ... test.
win:
	$(MAKE) PLATFORM=windows all

win-shared:
	$(MAKE) PLATFORM=windows shared

win-test:
	$(MAKE) PLATFORM=windows test

dbg: CFLAGS = $(CDBGFLAGS)
dbg: $(STATIC_LIBRARY)

shared: $(SHARED_LIBRARY)

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

mrproper:
	rm -rf "$(BUILD_ROOT)" "$(BUILD_DIR)" "$(STATIC_LIBRARY)" \
		"$(SHARED_LIBRARY)" "$(TEST_BINARY)" "$(TEST_LIBRARY)" "$(DOCS_DIR)"

# The flag files make changed compiler/linker options invalidate old outputs
# without rebuilding everything on every make invocation.
$(STATIC_FLAGS_FILE): FORCE
	@mkdir -p "$(@D)"
	@printf '%s\n' '$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS)' | cmp -s - "$@" || \
		printf '%s\n' '$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS)' > "$@"

$(SHARED_FLAGS_FILE): FORCE
	@mkdir -p "$(@D)"
	@printf '%s\n' '$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) $(PICFLAGS)' | cmp -s - "$@" || \
		printf '%s\n' '$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) $(PICFLAGS)' > "$@"

$(TEST_FLAGS_FILE): FORCE
	@mkdir -p "$(@D)"
	@printf '%s\n' '$(CC) $(BASE_CPPFLAGS) $(TEST_CPPFLAGS) $(BASE_CFLAGS)' | cmp -s - "$@" || \
		printf '%s\n' '$(CC) $(BASE_CPPFLAGS) $(TEST_CPPFLAGS) $(BASE_CFLAGS)' > "$@"

$(TEST_LIBRARY_FLAGS_FILE): FORCE
	@mkdir -p "$(@D)"
	@printf '%s\n' '$(CC) $(BASE_CPPFLAGS) -DJALLOC_UNITTESTING $(BASE_CFLAGS)' | cmp -s - "$@" || \
		printf '%s\n' '$(CC) $(BASE_CPPFLAGS) -DJALLOC_UNITTESTING $(BASE_CFLAGS)' > "$@"

$(STATIC_ARCHIVE_FLAGS_FILE): FORCE
	@mkdir -p "$(@D)"
	@printf '%s\n' '$(AR) $(ARFLAGS)' | cmp -s - "$@" || \
		printf '%s\n' '$(AR) $(ARFLAGS)' > "$@"

$(TEST_ARCHIVE_FLAGS_FILE): FORCE
	@mkdir -p "$(@D)"
	@printf '%s\n' '$(AR) $(ARFLAGS)' | cmp -s - "$@" || \
		printf '%s\n' '$(AR) $(ARFLAGS)' > "$@"

$(LINK_FLAGS_FILE): FORCE
	@mkdir -p "$(@D)"
	@printf '%s\n' '$(CC) $(LDFLAGS) $(LDLIBS) $(SHARED_PLATFORM_FLAGS) $(SHARED_IMPORT_LIBRARY)' | cmp -s - "$@" || \
		printf '%s\n' '$(CC) $(LDFLAGS) $(LDLIBS) $(SHARED_PLATFORM_FLAGS) $(SHARED_IMPORT_LIBRARY)' > "$@"

$(BUILD_DIR)/static/%.o: src/%.c $(STATIC_FLAGS_FILE)
	@mkdir -p "$(@D)"
	$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/shared/%.o: src/%.c $(SHARED_FLAGS_FILE)
	@mkdir -p "$(@D)"
	$(CC) $(BASE_CPPFLAGS) $(BASE_CFLAGS) $(PICFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/test/%.o: test/%.c $(TEST_FLAGS_FILE)
	@mkdir -p "$(@D)"
	$(CC) $(BASE_CPPFLAGS) $(TEST_CPPFLAGS) $(BASE_CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/test-lib/%.o: src/%.c $(TEST_LIBRARY_FLAGS_FILE)
	@mkdir -p "$(@D)"
	$(CC) $(BASE_CPPFLAGS) -DJALLOC_UNITTESTING $(BASE_CFLAGS) -MMD -MP -c $< -o $@

$(STATIC_LIBRARY): $(STATIC_OBJECTS) $(STATIC_ARCHIVE_FLAGS_FILE)
	@mkdir -p "$(@D)"
	rm -f "$@"
	$(AR) $(ARFLAGS) $@ $(STATIC_OBJECTS)

$(TEST_LIBRARY): $(TEST_LIBRARY_OBJECTS) $(TEST_ARCHIVE_FLAGS_FILE)
	@mkdir -p "$(@D)"
	rm -f "$@"
	$(AR) $(ARFLAGS) $@ $(TEST_LIBRARY_OBJECTS)

$(TEST_BINARY): $(TEST_OBJECTS) $(TEST_LIBRARY) $(LINK_FLAGS_FILE)
	@mkdir -p "$(@D)"
	$(CC) $(LDFLAGS) -o $@ $(TEST_OBJECTS) $(TEST_LIBRARY) $(LDLIBS) -lm

ifeq ($(PLATFORM),posix)
$(SHARED_EXPORTS): Makefile
	@mkdir -p "$(@D)"
	@printf '%s\n' '{' '  global:' '    jalloc;' '    jcalloc;' '    jrealloc;' '    jfree;' '  local:' '    *;' '};' > "$@"
else
$(SHARED_EXPORTS): Makefile
	@mkdir -p "$(@D)"
	@printf '%s\n' 'LIBRARY jalloc' 'EXPORTS' '    jalloc' '    jcalloc' '    jrealloc' '    jfree' > "$@"
endif

$(SHARED_LIBRARY): $(SHARED_OBJECTS) $(SHARED_EXPORTS) $(LINK_FLAGS_FILE)
	@mkdir -p "$(@D)"
	$(CC) -shared $(LDFLAGS) $(SHARED_PLATFORM_FLAGS) -o $@ \
		$(SHARED_LINKER_INPUTS) $(SHARED_OBJECTS) $(LDLIBS)

-include $(DEPFILES)

FORCE:
