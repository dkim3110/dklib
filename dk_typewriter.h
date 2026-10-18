/*
# `dk_typewriter.h` - public domain - Daniel Inhoi Kim, 2026
Header-only library for a typewriter style print function + cross-platform high-resolution sleep function. See end of
file for license information.

## Usage
Macros that enable function definitions (excluding the MAKE_STATIC macros) must only be defined once in one source file
before including the header. See
https://github.com/nothings/stb/blob/f58f558c120e9b32c217290b80bad1a0729fbb2c/docs/stb_howto.txt for more information.

This library requires GCC or Clang with GNU C extensions, so compile with `-std=gnu11`.

This library does not concern itself with C++ support.

### Flags
- DKTW_IMPLEMENTATION: enable function definitions
- DKTW_IMPLEMENTATION_MAKE_STATIC: same as `DKTW_IMPLEMENTATION` but all functions are static inline

### Example Code
  ```C
  #define DKTW_IMPLEMENTATION
  #include "dk_typewriter.h"

  #include <stdlib.h>
  #include <time.h>

  int main(void) {
    srand(time(NULL));

    dktw_typewriter(stdout, 1, "The quick brown fox jumps over the lazy dog.\n");
    dktw_sleep(5 DKTW_SECONDS);
    dktw_typewriter(stdout, 1, "5 seconds have passed.\n");

    return 0;
  }
  ```
 */

#ifdef DKTW_IMPLEMENTATION_MAKE_STATIC
  #define DKTW_DEC static inline
  #define DKTW_DEF DKTW_DEC

  #define DKTW_IMPLEMENTATION
#else
  #define DKTW_DEC extern
  #define DKTW_DEF
#endif // DKTW_IMPLEMENTATION_MAKE_STATIC

#ifndef DK_TYPEWRITER_H_
#define DK_TYPEWRITER_H_

#include <stdbool.h>
#include <stdio.h>

#define DKTW_NANOSECONDS  *1e-9
#define DKTW_MICROSECONDS *1e-6
#define DKTW_MILLISECONDS *1e-3
#define DKTW_SECONDS
#define DKTW_MINUTES *60
#define DKTW_HOURS   *3600

DKTW_DEC int dktw_sleep(double sec);
DKTW_DEC bool dktw_typewriter(FILE *stream, double speed, const char *s);

#endif // DK_TYPEWRITER_H_

#if defined(DKTW_IMPLEMENTATION) && !defined(DKTW_IMPLEMENTATION_GUARD)
#define DKTW_IMPLEMENTATION_GUARD

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif // WIN32_LEAN_AND_MEAN

  #include <windows.h>
#else
  #include <time.h>
#endif

static inline bool hlpr_dktw__is_pause(unsigned char c) {
  switch (c) {
    case ';':  /* fallthrough */
    case ':':  /* fallthrough */
    case ')':  /* fallthrough */
    case ']':  /* fallthrough */
    case '}':  /* fallthrough */
    case '\'': /* fallthrough */
    case '\"': /* fallthrough */
    case ',':  return true;
    default:   return false;
  }
}

static inline bool hlpr_dktw__is_stop(unsigned char c) {
  switch (c) {
    case '.': /* fallthrough */
    case '?': /* fallthrough */
    case '!': return true;
    default:  return false;
  }
}

DKTW_DEF int dktw_sleep(double sec) {
  if (sec < 0) {
    errno = EINVAL;
    return -1;
  }

#ifdef _WIN32
  Sleep((DWORD)(sec * 1e3));
  return 0;
#else
  int result;

  struct timespec ts;
  ts.tv_sec = sec;
  ts.tv_nsec = (sec - (double)ts.tv_sec) * 1e9;

  do {
    result = nanosleep(&ts, &ts);
  } while ((result) && (errno == EINTR));

  return result;
#endif // _WIN32
}

DKTW_DEF bool dktw_typewriter(FILE *stream, double speed, const char *s) {
  if ((!stream) || (!s)) return false;

  if (speed < 0.1) speed = 0.1;
  else if (speed > 10.0) speed = 10.0;

  for (size_t n = 0; s[n] != '\0'; n++) {
    fputc(s[n], stream);
    fflush(stream);

    if ((s[n + 1] == '\0') || (isspace((unsigned char)s[n + 1]))) {
      if (hlpr_dktw__is_stop((unsigned char)s[n])) dktw_sleep((383 DKTW_MILLISECONDS) / speed);
      else if (hlpr_dktw__is_pause((unsigned char)s[n])) dktw_sleep((153 DKTW_MILLISECONDS) / speed);
    }

    dktw_sleep(((70.0 + rand() % 16) DKTW_MILLISECONDS) / speed);
  }

  return true;
}

#endif // DKTW_IMPLEMENTATION

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2026 Daniel Inhoi Kim
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
