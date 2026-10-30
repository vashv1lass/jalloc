BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = test
TEST_THIRDPARTY_DIR = ${TEST_DIR}/third-party
DOCS_DIR = docs

SRC = ${SRC_DIR}/jalloc.c \
		${SRC_DIR}/logic/buddy.c \
		${SRC_DIR}/logic/segregated.c \
		${SRC_DIR}/os/mem.c

TEST_SRC = ${TEST_DIR}/run.c \
		${TEST_DIR}/testjalloc.c \
		${TEST_DIR}/logic/testbuddy.c \
		${TEST_DIR}/logic/testsegr.c \
		${TEST_DIR}/logic/testutils.c \
		${TEST_DIR}/os/testmem.c \
		${TEST_DIR}/os/testutils.c

PLATFORM ?= POSIX

ifeq (${PLATFORM},POSIX)

	CC ?= gcc # clang for MacOS (symlink)
	AR ?= ar
	ARFLAGS = rcs

	CPPFLAGS ?= -I${INCLUDE_DIR} -I${SRC_DIR} -D_POSIX_C_SOURCE=200809L
	CFLAGS ?= -std=c17 -Wall -Wextra -Wpedantic -Werror -fPIC
	LDFLAGS ?=
	LDLIBS ?=

	LIB_STATIC = libjalloc.a

	ifeq (${shell uname -s},Darwin)
		SHARED_FLAGS = -dynamiclib
		LIB_SHARED = libjalloc.dylib
	else
		SHARED_FLAGS = -shared
		LIB_SHARED = libjalloc.so
	endif

	OBJ = ${addprefix ${BUILD_DIR}/, ${SRC:.c=.o}}

	TEST_CPPFLAGS = ${CPPFLAGS} -I${TEST_DIR} -I${TEST_THIRDPARTY_DIR} -DJALLOC_UNITTESTING
	TEST_CFLAGS = ${CFLAGS} -I${TEST_DIR} -I${TEST_THIRDPARTY_DIR}
	TEST_LDFLAGS = ${LDFLAGS}
	TEST_LDLIBS = ${LDLIBS} -lm

	TEST_LIB_OBJ = ${addprefix ${BUILD_DIR}/test_lib/, ${SRC:.c=.o}}
	TEST_LIB_STATIC = ${BUILD_DIR}/libjalloc_test.a

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

else ifeq (${PLATFORM},Windows)

	# MSVC
	SHELL = cmd.exe
	.SHELLFLAGS = /c

	CC = cl
	AR = lib
	LD = link

	CPPFLAGS = /I${INCLUDE_DIR} /I${SRC_DIR}
	CFLAGS = /std:c17 /W4 /WX /nologo
	LDFLAGS =
	LDLIBS =

	SHARED_CPPFLAGS = /DJALLOC_BUILD_SHARED

	LIB_STATIC = jalloc.lib
	LIB_SHARED = jalloc.dll
	LIB_IMPORT = jalloc.lib
	LIB_EXPORT = jalloc.exp

	OBJ = ${addprefix ${BUILD_DIR}/, ${SRC:.c=.obj}}

	# MSVC does not like greatest, so i need to define D_CRT_SECURE_NO_WARNINGS
	TEST_CPPFLAGS = ${CPPFLAGS} /I${TEST_DIR} /I${TEST_THIRDPARTY_DIR} /DJALLOC_UNITTESTING /D_CRT_SECURE_NO_WARNINGS
	TEST_CFLAGS = ${CFLAGS}

	TEST_LIB_OBJ = ${addprefix ${BUILD_DIR}/test_lib/, ${SRC:.c=.obj}}
	TEST_LIB_STATIC = ${BUILD_DIR}\jalloc_test.lib

	TEST_OBJ = ${addprefix ${BUILD_DIR}/, ${TEST_SRC:.c=.obj}}
	TEST_BIN = ${BUILD_DIR}\testjalloc.exe

${LIB_STATIC}: ${OBJ}
	if not exist ${subst /,\,${dir $@}} mkdir ${subst /,\,${dir $@}}
	${AR} /OUT:$@ $^

shared: ${LIB_SHARED}

${LIB_SHARED}: CPPFLAGS += ${SHARED_CPPFLAGS}

${LIB_SHARED}: ${OBJ}
	if not exist ${subst /,\,${dir $@}} mkdir ${subst /,\,${dir $@}}
	${LD} /DLL /OUT:${LIB_SHARED} /IMPLIB:${LIB_IMPORT} $^
	if exist ${LIB_EXPORT} move /Y ${LIB_EXPORT} ${BUILD_DIR}\${LIB_EXPORT}

${BUILD_DIR}/test_lib/%.obj: %.c
	if not exist ${subst /,\,${dir $@}} mkdir ${subst /,\,${dir $@}}
	${CC} ${TEST_CPPFLAGS} ${TEST_CFLAGS} /c $< /Fo:$@

${BUILD_DIR}/${TEST_DIR}/%.obj: ${TEST_DIR}/%.c
	if not exist ${subst /,\,${dir $@}} mkdir ${subst /,\,${dir $@}}
	${CC} ${TEST_CPPFLAGS} ${TEST_CFLAGS} /c $< /Fo:$@

${BUILD_DIR}/%.obj: %.c
	if not exist ${subst /,\,${dir $@}} mkdir ${subst /,\,${dir $@}}
	${CC} ${CPPFLAGS} ${CFLAGS} /c $< /Fo:$@

${TEST_LIB_STATIC}: ${TEST_LIB_OBJ}
	if not exist ${subst /,\,${dir $@}} mkdir ${subst /,\,${dir $@}}
	${AR} /OUT:$@ $^

test: ${TEST_BIN}
	$<

${TEST_BIN}: ${TEST_OBJ} ${TEST_LIB_STATIC}
	if not exist ${subst /,\,${dir $@}} mkdir ${subst /,\,${dir $@}}
	${LD} /OUT:$@ $^ ${TEST_LIB_STATIC}

mrproper:
	if exist ${BUILD_DIR} rmdir /S /Q ${BUILD_DIR}
	if exist ${LIB_STATIC} del /Q ${LIB_STATIC}
	if exist ${LIB_SHARED} del /Q ${LIB_SHARED}
	if exist ${DOCS_DIR} rmdir /S /Q ${DOCS_DIR}

.PHONY: shared mrproper test

endif

DOXYGEN ?= doxygen
DOXYFILE ?= Doxyfile

docs:
	${DOXYGEN} ${DOXYFILE}

.PHONY: docs
