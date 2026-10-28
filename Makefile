BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = test
TEST_THIRDPARTY_DIR = ${TEST_DIR}/third-party
DOCS_DIR = docs

PLATFORM ?= POSIX

CC ?= gcc # clang for MacOS (symlink)
AR ?= ar
DOXYGEN ?= doxygen

CPPFLAGS ?= -I${INCLUDE_DIR} -I${SRC_DIR}
CFLAGS ?= -std=c17 -Wall -Wextra -Wpedantic -Werror
LDFLAGS ?=
LDLIBS ?=

ARFLAGS ?= rcs

DOXYFILE ?= Doxyfile

ifeq (${PLATFORM},POSIX)
	CPPFLAGS += -D_POSIX_C_SOURCE=200809L
	CFLAGS += -fPIC

	LIB_STATIC = libjalloc.a

	ifeq (${shell uname -s},Darwin)
		SHARED_FLAGS = -dynamiclib
		LIB_SHARED = libjalloc.dylib
	else
		SHARED_FLAGS = -shared
		LIB_SHARED = libjalloc.so
	endif

	SRC = ${SRC_DIR}/jalloc.c \
	      ${SRC_DIR}/logic/buddy.c \
	      ${SRC_DIR}/logic/segregated.c \
	      ${SRC_DIR}/os/mem.c
	OBJ = ${addprefix ${BUILD_DIR}/, ${SRC:.c=.o}}

	TEST_CPPFLAGS = ${CPPFLAGS} -I${TEST_DIR} -I${TEST_THIRDPARTY_DIR} -DJALLOC_UNITTESTING
	TEST_CFLAGS = ${CFLAGS} -I${TEST_DIR} -I${TEST_THIRDPARTY_DIR}
	TEST_LDFLAGS = ${LDFLAGS}
	TEST_LDLIBS = ${LDLIBS} -lm

	TEST_LIB_OBJ = ${addprefix ${BUILD_DIR}/test_lib/, ${SRC:.c=.o}}
	TEST_LIB_STATIC = ${BUILD_DIR}/libjalloc_test.a

	TEST_SRC = ${TEST_DIR}/run.c \
		   ${TEST_DIR}/testjalloc.c \
		   ${TEST_DIR}/logic/testbuddy.c \
		   ${TEST_DIR}/logic/testsegr.c \
		   ${TEST_DIR}/logic/testutils.c \
		   ${TEST_DIR}/os/testmem.c \
		   ${TEST_DIR}/os/testutils.c
	TEST_OBJ = ${addprefix ${BUILD_DIR}/, ${TEST_SRC:.c=.o}}
	TEST_BIN = ${BUILD_DIR}/testjalloc

${LIB_STATIC}: ${OBJ}
	@mkdir -p ${dir $@}
	${AR} ${ARFLAGS} $@ $^

shared: ${LIB_SHARED}

${LIB_SHARED}: ${OBJ}
	@mkdir -p ${dir $@}
	${CC} ${SHARED_FLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

${BUILD_DIR}/test_lib/%.o: %.c
	@mkdir -p ${dir $@}
	${CC} ${TEST_CPPFLAGS} ${TEST_CFLAGS} -c $< -o $@

${BUILD_DIR}/${TEST_DIR}/%.o: ${TEST_DIR}/%.c
	@mkdir -p ${dir $@}
	${CC} ${TEST_CPPFLAGS} ${TEST_CFLAGS} -c $< -o $@

${BUILD_DIR}/%.o: %.c
	@mkdir -p ${dir $@}
	${CC} ${CPPFLAGS} ${CFLAGS} -c $< -o $@

${TEST_LIB_STATIC}: ${TEST_LIB_OBJ}
	@mkdir -p ${dir $@}
	${AR} ${ARFLAGS} $@ $^

test: ${TEST_BIN}
	$<

${TEST_BIN}: ${TEST_OBJ} ${TEST_LIB_STATIC}
	@mkdir -p ${dir $@}
	${CC} ${TEST_LDFLAGS} -o $@ $^ ${TEST_LDLIBS}

mrproper:
	rm -rf ${BUILD_DIR} ${LIB_STATIC} ${LIB_SHARED} ${DOCS_DIR}

.PHONY: shared mrproper test

else # ifeq (${PLATFORM},Windows)
	# TODO: add windows later
endif

docs:
	@if [ -f "${DOXYFILE}" ]; then \
		${DOXYGEN} ${DOXYFILE}; \
	else \
		echo "${DOXYFILE} not found. Run '${DOXYGEN} -g' to generate a default config."; \
	fi

.PHONY: docs
