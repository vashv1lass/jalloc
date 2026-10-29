# jalloc

## What is jalloc?

A lightweight memory allocator written in C using strictly C17 and POSIX.1-2008
(IEEE 1003.1-2008) (or WinAPI/UCRT) standards built around the buddy allocation
algorithm. It is a small project that implements `malloc`-like dynamic memory
allocation via custom memory allocation logic and memory mapping.

## How to use it in my project?

### Requirements

#### Mandatory

- C17-compatible compiler that supports `uintptr_t` (your compiler likely supports)
- Flat memory model (your system likely satisfies this condition)
- GNU make (or MinGW make)
- Windows or POSIX-compatible operating system (some kinds of embedded systems
like FreeRTOS are not supported). Also system must have page size, that is
power of 2 (99.9% systems satisfy this condition).

##### Windows

- Make sure Microsoft Visual Studio is installed (project is compiled via MSVC
cl.exe)
- You will need to build the project from the Developer Command Prompt

#### Optional

- Doxygen (for `make docs`)

### Configuring

Configuring is suckless-like. You are able to configure the allocator in
`src/cfg/conf.h` file. You can configure:
- `JALLOC_MINORDER` parameter.
- `JALLOC_MAXORDER` parameter.
- `JALLOC_CHUNKSZ` parameter.

Make sure that values of these parameters satisfies the static assertions.

### Building

Build the static library:

```bash
make # Generates .a (POSIX) static library in the root of the project
make PLATFORM=Windows # Generates .lib (Windows) static library in the root of
                      # the project
```

Build the shared library:

```bash
make shared # Generates .so (Linux/BSD) or .dylib (Darwin) shared (i.e.
            # "dynamic") library in the root of the project

mingw32-make PLATFORM=Windows shared # Generates .dll (Windows) dynamic
                                     # link (i.e. "shared") library in
                                     # the root of the project
```

Run the test suite:

```bash
make test # Builds .a and runs tests (POSIX)

mingw32-make PLATFORM=Windows test # Builds .lib and runs tests (Windows)
```

Generate the API documentation:

```bash
make docs # or mingw32-make PLATFORM=Windows docs on Windows
```

Remove the build artifacts:

```bash
make mrproper # or mingw32-make PLATFORM=Windows mrproper on Windows
```

### Including to the project

To include `jalloc` to your project, copy `include/jalloc.h` file to your
include directory and the generated library file (i.e. `.a`/`.lib` or
`.so`/`dylib`/`.dll`) to your library directory.

### Usage

Include the copied public header:

```c
#include <jalloc.h> // make sure you add include directory (like -Iinclude)
```

### Example

```c
#include <jalloc.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

int
main(void)
{
    size_t bufsz = 32;
    char * buf   = (char *)jalloc(bufsz);

    /*
     * jalloc behaviour is similar to malloc behaviour, so you also need to check
     * for allocation errors
     */
    if (buf == NULL) {
        perror("jalloc error :(");
        return 1;
    }

    memcpy(
        buf,
        "thank you for using jalloc! :)",
        sizeof("thank you for using jalloc! :)")
    );

    /* jrealloc is also similar to default realloc */
    char * resized = (char *)jrealloc(buf, 64);
    if (resized == NULL) {
        /* the original allocation remains valid */
        jfree(buf);
        perror("jrealloc error :(");
        return 1;
    }
    buf = resized;

    puts(buf);

    /* jfree is similar to standard free */
    jfree(buf);

    return 0;

}
```

### Compiling, linking and running

For example, let the platform be POSIX-compatible, `jalloc.h` be in `include/`
directory, `libjalloc.a` be in `lib/` directory, example source code file has
the name of `example.c` and the project is compiled via `gcc`.

```bash
gcc -std=c17 -Iinclude example.c lib/libjalloc.a -o example
```

## Design

The allocator is organized into several layers:

- Public API - exposes allocation and deallocation functions.
- Buddy - manages block splitting and coalescing.
- Segregated - tracks available blocks by the size order.
- OS memory - obtains memory mappings.
- Configuration and layout - defines block metadata, alignment and allocator limits.

## Current limitaions

- The allocator uses a fixed range of supported block sizes.
- A single allocation cannot exceed the configured maximum payload size.
- No thread-safety.
- Allocator does not return memory to OS (not a leak, but bad memory
utilization)

## Development vector

- Chunk tracking and returning memory to OS.
- Memory leak detection.
- Benchmarking.
- Thread-safety.

## License

This project is licensed under the BSD-3-Clause license. See
[LICENSE](./LICENSE) for details.

## Author

Maksim Vashkevich
