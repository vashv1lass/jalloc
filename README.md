# jalloc

## What is jalloc?

A lightweight memory allocator written in C using strictly C17 and POSIX.1-2008
(IEEE 1003.1-2008) standards built around the buddy allocation algorithm.
It is a small project that implements `malloc`-like dynamic memory allocation
via custom memory allocation logic and memory mapping.

## How to use it in my project?

### Requirements

#### Mandatory

- C17-compatible compiler
- GNU make
- Linux/BSD operating system (for now)

#### Optional

- Doxygen

### Building

Build the static library (`.a` for now):

```bash
make
```

Build the shared library (`.so` for now):

```bash
make shared
```

Run the test suite:

```bash
make test
```

Generate the API documentation:

```bash
make docs
```

Remove the build artifacts:

```bash
make mrproper
```

### Including to the project

To include `jalloc` to your project, copy `include/jalloc.h` file to your
include directory and the generated library file (i.e. `.a` or `.so`) to your
library directory.

### Usage

Include the copied public header:

```c
#include <jalloc.h>
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

For example, let `jalloc.h` be in `include/` directory, `libjalloc.a` be in
`lib/` directory, example source code file has the name of `example.c` and the
project is compiled via `gcc`.

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
- The implementation relies on POSIX memory-mapping facilities.

## Development vector

- Port allocator to Windows (WinAPI/UCRT).
- Memory leak detection.
- Configurability.
- Benchmarking.

## License

This project is licensed under the BSD-3-Clause license. See
[LICENSE](./LICENSE) for details.

## Author

Maksim Vashkevich
