# jalloc

## What is jalloc?

A lightweight memory allocator written in C using strictly C17 and POSIX.1-2008
(IEEE 1003.1-2008) (or WinAPI/UCRT) standards built around the buddy allocation
algorithm. It is a small project that implements `malloc`-like dynamic memory
allocation via custom memory allocation logic and memory mapping.

## How to use it in my project?

### Requirements

#### Mandatory

##### Overall

- C17-compatible compiler that supports `uintptr_t` (your C17 compiler likely
  supports).
- Flat memory model (your system likely satisfies this condition)
- GNU make (or MinGW make for Windows)
- Windows or POSIX-compatible operating system (some kinds of embedded systems
  like FreeRTOS are not supported). Also system must have page size, that is
  power of 2 (your system also likely satisfies this condition).
- Byte must be equal 8 bits (`CHAR_BIT == 8`) without any padding bits.
  Your system likely satisfies this condition.

##### POSIX-specific

- Your POSIX-system must support Shared Memory Objects / Realtime (your system
  likely supports it).
- (Linux only): your Linux system must have realtime library for it (-lrt)
  (in practice, your system either has it or even does not strictly require it).
- Maximum length of the file name on your system must be at least 35
  characters (it may be strict 14, as IEEE 1003.1-2008 says and in practice,
  everything should (and will) work perfectly on such systems, but purely from
  mathematical point of view, 100% confidence is achieved with at least 35
  characters). Also, your system likely has 255 characters limitation, so this
  restriction most likely does not apply to you.
- C17 compiler must support such GNU/Clang-like flags as -fPIC and -fvisibility.

##### Windows-specific

- Make sure Microsoft Visual Studio is installed (project is compiled via MSVC
  cl.exe)
- You will need to build the project from the Developer Command Prompt or add
  cl.exe to `PATH`

#### Optional

- Doxygen (for `make docs`)

### Configuring

Configuring is suckless-like. You are able to configure the allocator in
`src/cfg/conf.h` file. You are able to configure the following:

- `JALLOC_MINORDER` parameter.
- `JALLOC_MAXORDER` parameter.
- `JALLOC_CHUNKMULT` parameter.

Make sure that values of these parameters satisfies the static assertions.

### Building

> [!NOTE]
> If you want to rebuild your project, make sure you have called `mrproper` before.

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

mingw32-make PLATFORM=Windows shared # Generates .dll (Windows) dynamic link
                                     # (i.e. "shared") library and .lib (for
                                     # your project) in the root.
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

#### Static library

##### POSIX-compatible platforms

For example, let the project structure be like that:

```text
example/
├── include/
│   └── jalloc.h
├── lib/
│   └── libjalloc.a
└── example.c
```

Then, you can compile your program like that:

```bash
gcc -std=c17 -Iinclude example.c lib/libjalloc.a -o example -lrt
```

> [!NOTE]
> `-lrt` is likely not neccessary and also (on Darwin systems like MacOS) may be
> even extraneous.

##### Windows platforms

> [!WARNING]
> I do NOT recommend this path, use compile your project with jalloc imported as
> .dll instead!

Let the project structure be the same, but instead of `libjalloc.a` in the
`lib/` directory, we have a `jalloc.lib` library in it.

On Windows, the usage of static libraries is not safe due to `.lib` file
structure (it exposes the private API symbols). Nevertheless, to build the
project with Windows static library, you should do the following in the
Developer Command Prompt (or add cl.exe in `PATH`):

```powershell
cl /std:c17 /Iinclude example.c lib\jalloc.lib /Fe:example.exe
```

#### Shared (dynamic) library

##### POSIX compatible platforms

###### Darwin (MacOS)

Let the project structure also be the same, but in the `lib/` directory, there's
a `libjalloc.dylib` library in it. Also the compiler will be clang, but on Mac,
by default, `gcc` is just a symlink for `clang`, so there's no difference:

```zsh
clang -std=c17 -Iinclude example.c lib/libjalloc.dylib -o example \
           -Wl,-rpath,@loader_path/lib
```

###### Other POSIX-compatible systems (like Linux)

On other POSIX systems, with `.so` file in `lib/` directory, the project
building algorithm will be the following:

```bash
gcc -std=c17 -Iinclude example.c lib/libjalloc.so -o example \
         -Wl,-rpath,'$ORIGIN/lib' -lrt
```

##### Windows systems

To build your project on Windows via importing `jalloc` as a dynamic link
library, put the `.dll` file in the directory, where the `.exe` file will be
located, and `.lib` file in `lib/` directory. Then do the following:

```powershell
cl /std:c17 /Iinclude example.c lib\jalloc.lib /Fe:test.exe
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
