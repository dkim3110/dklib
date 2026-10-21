/*
# `dk_strview.h` - public domain - Daniel Inhoi Kim, 2026
Header-only library for string views. See end of file for license information.

## Usage
Macros that enable function definitions (excluding the MAKE_STATIC macros) must only be defined once in one source file
before including the header. See
https://github.com/nothings/stb/blob/f58f558c120e9b32c217290b80bad1a0729fbb2c/docs/stb_howto.txt for more information.

This library requires GCC or Clang with GNU C extensions, so compile with `-std=gnu11`. Additionally, this library
includes `<math.h>`, so compile with `-lm` as well.

This library does not concern itself with C++ support.

### Flags
- DKSV_IMPLEMENTATION: enable function definitions
- DKSV_IMPLEMENTATION_MAKE_STATIC: same as `DKSV_IMPLEMENTATION` but all functions are static inline

### Example Code
  ```C
  #define DKSV_IMPLEMENTATION
  #include "dk_strview.h"

  #include <stdio.h>

  int main(void) {
    dksv text = dksv_view("  Hello, World!  ");
    dksv_print(dksv_trim(text));
  }
  ```
 */

#ifdef DKSV_IMPLEMENTATION_MAKE_STATIC
  #define DKSV_DEC static inline
  #define DKSV_DEF DKSV_DEC

  #define DKSV_IMPLEMENTATION
#else
  #define DKSV_DEC extern
  #define DKSV_DEF
#endif // DKSV_IMPLEMENTATION_MAKE_STATIC

#ifndef DK_STR_VIEW_H_
#define DK_STR_VIEW_H_

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE
#endif //_GNU_SOURCE

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *data;
  size_t len;
} dksv;

#define DKSV_NULL    ((dksv){.data = NULL, .len = 0})
#define DKSV_FMT     "%.*s"
#define DKSV_ARG(sv) (int)(sv).len, (sv).data

#define dksv_view(str) dksv_view_wlen((str), strlen(str))

#ifdef DKA_STRING_BUILDER_GUARD
  #define dksb_to_dksv(sb) dksv_view_wlen((sb), dksb_strlen(sb))
#endif // DKA_STRING_BUILDER_GUARD

DKSV_DEC bool dksv_isempty(dksv sv);
DKSV_DEC bool dksv_isblank(dksv sv);
DKSV_DEC bool dksv_equals(dksv a, dksv b);
DKSV_DEC bool dksv_ic_equals(dksv a, dksv b);
DKSV_DEC int dksv_cmp(dksv a, dksv b);
DKSV_DEC bool dksv_isalpha(dksv sv);
DKSV_DEC bool dksv_isalnum(dksv sv);
DKSV_DEC bool dksv_isdigit(dksv sv);

DKSV_DEC void dksv_fput(dksv sv, FILE *stream);
DKSV_DEC void dksv_print(dksv sv);
DKSV_DEC void dksv_println(dksv sv);

DKSV_DEC dksv dksv_take(dksv sv, size_t n);
DKSV_DEC dksv dksv_drop(dksv sv, size_t n);
DKSV_DEC dksv dksv_sub(dksv sv, size_t start, size_t end);
DKSV_DEC dksv dksv_split(dksv *sv, char delimiter);
DKSV_DEC dksv dksv_split_ws(dksv *sv);

DKSV_DEC dksv dksv_trim(dksv sv);
DKSV_DEC dksv dksv_ltrim(dksv sv);
DKSV_DEC dksv dksv_rtrim(dksv sv);
DKSV_DEC dksv dksv_trimc(dksv sv, char c);
DKSV_DEC dksv dksv_ltrimc(dksv sv, char c);
DKSV_DEC dksv dksv_rtrimc(dksv sv, char c);

DKSV_DEC bool dksv_prefix(dksv base, dksv prefix);
DKSV_DEC bool dksv_suffix(dksv base, dksv suffix);
DKSV_DEC bool dksv_ic_prefix(dksv base, dksv prefix);
DKSV_DEC bool dksv_ic_suffix(dksv base, dksv suffix);
DKSV_DEC bool dksv_contains(dksv base, dksv item);
DKSV_DEC int64_t idx_dksv_contains(dksv base, dksv item);
DKSV_DEC int64_t dksv_idx(dksv sv, char c);
DKSV_DEC int64_t dksv_idx_last(dksv sv, char c);
DKSV_DEC int32_t dksv_char_at(dksv sv, size_t index);
DKSV_DEC size_t dksv_count_char(dksv sv, char c);

DKSV_DEC dksv dksv_view_wlen(const char *string, size_t len);
DKSV_DEC double dksv_to_dbl(dksv sv);

#endif // DK_STR_VIEW_H_

#if defined(DKSV_IMPLEMENTATION) && !defined(DKSV_IMPLEMENTATION_GUARD)
#define DKSV_IMPLEMENTATION_GUARD

#include <ctype.h>
#include <math.h>

/*
 * Function to verify if a given dksv struct is empty.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool dksv_isempty(dksv sv) {
  return ((sv.len == 0) || (!sv.data));
} /* dksv_isempty() */

/*
 * Function to verify if a given dksv struct is blank: either empty or consisting of only whitespaces.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool dksv_isblank(dksv sv) {
  if (dksv_isempty(sv)) return true;

  for (size_t n = 0; n < sv.len; n++) {
    if (!isspace((unsigned char)sv.data[n])) return false;
  }

  return true;
} /* dksv_isblank() */

/*
 * Function to verify if two dksv structs are equal.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool dksv_equals(dksv a, dksv b) {
  if (a.len != b.len) return false;
  if (dksv_isempty(a) || dksv_isempty(b)) return (dksv_isempty(a) == dksv_isempty(b));
  return (memcmp(a.data, b.data, a.len) == 0);
} /* dksv_equals() */

/*
 * Function to verify if two dksv structs are equal, ignoring letter case.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool dksv_ic_equals(dksv a, dksv b) {
  if (a.len != b.len) return false;
  if (dksv_isempty(a) || dksv_isempty(b)) return (dksv_isempty(a) == dksv_isempty(b));

  for (size_t n = 0; n < a.len; n++) {
    if (toupper((unsigned char)a.data[n]) != toupper((unsigned char)b.data[n])) return false;
  }

  return true;
} /* dksv_ic_equals() */

/*
 * Function to compare two strings.
 * Returns 0 if a and b are equal, a negative value if a is less than b,
 * and a positive value if a is greater than b.
 */

DKSV_DEF int dksv_cmp(dksv a, dksv b) {
  size_t cmp_len = (a.len < b.len) ? a.len : b.len;
  int cmp = (cmp_len) ? memcmp(a.data, b.data, cmp_len) : 0;

  if (cmp) return cmp;
  if (a.len != b.len) return (a.len < b.len) ? -1 : 1;
  return 0;
} /* dksv_cmp() */

/*
 * Function to determine if a given dksv struct consists of only alphabet characters.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool dksv_isalpha(dksv sv) {
  if (dksv_isempty(sv)) return false;

  for (size_t n = 0; n < sv.len; n++) {
    if (!isalpha((unsigned char)sv.data[n])) return false;
  }

  return true;
} /* dksv_isalpha() */

/*
 * Function to determine if a given dksv struct consists of only alphabet characters
 * and digits.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool dksv_isalnum(dksv sv) {
  if (dksv_isempty(sv)) return false;

  for (size_t n = 0; n < sv.len; n++) {
    if (!isalnum((unsigned char)sv.data[n])) return false;
  }

  return true;
} /* dksv_isalnum() */

/*
 * Function to determine if a given dksv struct consists of only digits.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool dksv_isdigit(dksv sv) {
  if (dksv_isempty(sv)) return false;

  for (size_t n = 0; n < sv.len; n++) {
    if (!isdigit((unsigned char)sv.data[n])) return false;
  }

  return true;
} /* dksv_isdigit() */

/*
 * Function to write a dksv struct to a specified file stream.
 */

DKSV_DEF void dksv_fput(dksv sv, FILE *stream) {
  if ((!stream) || (dksv_isempty(sv))) return;
  fwrite(sv.data, sizeof(char), sv.len, stream);
} /* dksv_fput() */

/*
 * Function to print a dksv struct to stdout.
 */

DKSV_DEF void dksv_print(dksv sv) {
  dksv_fput(sv, stdout);
} /* dksv_print() */

/*
 * Function to print a dksv struct to stdout, then a newline character.
 */

DKSV_DEF void dksv_println(dksv sv) {
  dksv_print(sv);
  putchar('\n');
} /* dksv_println() */

/*
 * Function to take characters from a given dksv struct up to the specified index.
 * It is exclusive and thus will not take the character at index n.
 * Returns sv unmodified if empty.
 */

DKSV_DEF dksv dksv_take(dksv sv, size_t n) {
  if (dksv_isempty(sv)) return sv;
  if (n > sv.len) n = sv.len;

  return (dksv){.data = sv.data, .len = n};
} /* dksv_take() */

/*
 * Function to drop characters from a dksv struct up to the specified index.
 * It is exclusive and thus will not drop the character at index n.
 * Returns sv unmodifed if empty.
 */

DKSV_DEF dksv dksv_drop(dksv sv, size_t n) {
  if (dksv_isempty(sv)) return sv;
  if (n > sv.len) n = sv.len;

  return (dksv){.data = sv.data + n, .len = sv.len - n};
} /* dksv_drop() */

/*
 * Function to extract a substring from a dksv struct from the start and end indexes provided.
 * Start is inclusive, end is exclusive.
 */

DKSV_DEF dksv dksv_sub(dksv sv, size_t start, size_t end) {
  return dksv_drop(dksv_take(sv, end), start);
} /* dksv_sub() */

/*
 * Function to split a given dksv struct based on the specifed delimiter.
 * Returns a dksv struct up to the first instance of the specified delimiter, or the original string
 * if it is empty. The original dksv struct retains all characters after the
 * delimiter; mutated by the function. The delimiter itself gets consumed.
 */

DKSV_DEF dksv dksv_split(dksv *sv, char delimiter) {
  if (dksv_isempty(*sv)) return *sv;

  const char *found = (const char *)memchr(sv->data, delimiter, sv->len);
  size_t n = (found) ? (size_t)(found - sv->data) : sv->len;

  dksv result = dksv_take(*sv, n);
  if (n < sv->len) *sv = dksv_drop(*sv, n + 1);
  else *sv = dksv_drop(*sv, n);

  return result;
} /* dksv_split() */

/*
 * Function to split a given dksv struct by whitespaces.
 * Returns a dksv struct up to the first instance of a whitespace, or the original string
 * if it is empty. The original dksv struct retains all characters after the
 * delimiter; mutated by the function. The delimiter itself gets consumed.
 */

DKSV_DEF dksv dksv_split_ws(dksv *sv) {
  if (dksv_isempty(*sv)) return *sv;

  size_t n = 0;
  while ((n < sv->len) && (!isspace((unsigned char)sv->data[n]))) n++;

  dksv result = dksv_take(*sv, n);
  if (n < sv->len) *sv = dksv_drop(*sv, n + 1);
  else *sv = dksv_drop(*sv, n);

  return result;
} /* dksv_split_ws() */

/*
 * Function to trim leading and trailing whitespace from a given dksv struct.
 * Returns original string if the given string is empty.
 */

DKSV_DEF dksv dksv_trim(dksv sv) {
  return dksv_ltrim(dksv_rtrim(sv));
} /* dksv_trim() */

/*
 * Function to trim leading whitespace from a given dksv struct.
 * Returns original string if the given string is empty.
 *
 */

DKSV_DEF dksv dksv_ltrim(dksv sv) {
  if (dksv_isempty(sv)) return sv;
  while ((sv.len > 0) && (isspace((unsigned char)sv.data[0]))) sv = dksv_drop(sv, 1);

  return sv;
} /* dksv_ltrim() */

/*
 * Function to trim trailing whitespace from a given dksv struct.
 * Returns original string if the given string is empty.
 */

DKSV_DEF dksv dksv_rtrim(dksv sv) {
  if (dksv_isempty(sv)) return sv;
  while ((sv.len > 0) && (isspace((unsigned char)sv.data[sv.len - 1]))) sv = dksv_take(sv, sv.len - 1);

  return sv;
} /* dksv_rtrim() */

/*
 * Function to trim leading and trailing instances of the provided character from
 * a given dksv struct.
 * Returns original string if the given string is empty.
 */

DKSV_DEF dksv dksv_trimc(dksv sv, char c) {
  return dksv_ltrimc(dksv_rtrimc(sv, c), c);
} /* dksv_trimc() */

/*
 * Function to trim leading instances of the provided character from a given dksv struct.
 * Returns original string if the given string is empty.
 */

DKSV_DEF dksv dksv_ltrimc(dksv sv, char c) {
  if (dksv_isempty(sv)) return sv;
  while ((sv.len > 0) && (sv.data[0] == c)) sv = dksv_drop(sv, 1);

  return sv;
} /* dksv_ltrimc() */

/*
 * Function to trim trailing instances of the provided character from a given dksv struct.
 * Returns original string if the given string is empty.
 */

DKSV_DEF dksv dksv_rtrimc(dksv sv, char c) {
  if (dksv_isempty(sv)) return sv;
  while ((sv.len > 0) && (sv.data[sv.len - 1] == c)) sv = dksv_take(sv, sv.len - 1);

  return sv;
} /* dksv_rtrimc() */

/*
 * Function to verify if a given dksv struct starts with the specified prefix.
 * Returns 1 if true, 0 if false. Empty strings are considered to be contained by
 * all strings.
 */

DKSV_DEF bool dksv_prefix(dksv base, dksv prefix) {
  if (dksv_isempty(prefix)) return true;
  if ((!base.data) || (!prefix.data)) return false;
  if ((prefix.len > base.len) || (dksv_isempty(base))) return false;
  return (memcmp(base.data, prefix.data, prefix.len) == 0);
} /* dksv_prefix() */

/*
 * Function to verify if a given dksv struct ends with the specified suffix.
 * Returns 1 if true, 0 if false. Empty strings are considered to be contained by
 * all strings.
 */

DKSV_DEF bool dksv_suffix(dksv base, dksv suffix) {
  if (dksv_isempty(suffix)) return true;
  if ((!base.data) || (!suffix.data)) return false;
  if ((suffix.len > base.len) || (dksv_isempty(base))) return false;
  return (memcmp(base.data + (base.len - suffix.len), suffix.data, suffix.len) == 0);
} /* dksv_suffix() */

/*
 * Function to verify if a given dksv struct starts with the specified prefix, ignoring letter case.
 * Returns 1 if true, 0 if false. Empty strings are considered to be contained by
 * all strings.
 */

DKSV_DEF bool dksv_ic_prefix(dksv base, dksv prefix) {
  if (dksv_isempty(prefix)) return true;
  if ((!base.data) || (!prefix.data)) return false;
  if ((prefix.len > base.len) || (dksv_isempty(base))) return false;
  return dksv_ic_equals(dksv_take(base, prefix.len), prefix);
} /* dksv_ic_prefix() */

/*
 * Function to verify if a given dksv struct ends with the specified suffix, ignoring letter case.
 * Returns 1 if true, 0 if false. Empty strings are considered to be contained by
 * all strings.
 */

DKSV_DEF bool dksv_ic_suffix(dksv base, dksv suffix) {
  if (dksv_isempty(suffix)) return true;
  if ((!base.data) || (!suffix.data)) return false;
  if ((suffix.len > base.len) || (dksv_isempty(base))) return false;
  return dksv_ic_equals(dksv_drop(base, base.len - suffix.len), suffix);
} /* dksv_ic_suffix() */

/*
 * Function to verify if a given dksv struct contains the specified string.
 * Empty strings are considered to be contained by all strings.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool dksv_contains(dksv base, dksv item) {
  return (idx_dksv_contains(base, item) != -1);
} /* dksv_contains() */

/*
 * Function to verify if a given dksv struct contains the specified string.
 * Empty strings are considered to be contained by all strings. Returns the index
 * of the first occurrence or -1 if the base dksv does not contain the item.
 */

DKSV_DEF int64_t idx_dksv_contains(dksv base, dksv item) {
  if (dksv_isempty(item)) return 0;
  if ((item.len > base.len) || (dksv_isempty(base))) return -1;

  char *sub = memmem(base.data, base.len, item.data, item.len);
  return (sub) ? sub - base.data : -1;
} /* idx_dksv_contains() */

/*
 * Function to identify the index within the dksv struct of the first occurrence
 * of the specified char.
 * Returns -1 if no matching characters are found.
 */

DKSV_DEF int64_t dksv_idx(dksv sv, char c) {
  if (dksv_isempty(sv)) return -1;
  const char *found = (const char *)memchr(sv.data, c, sv.len);
  return (found) ? found - sv.data : -1;
} /* dksv_idx() */

/*
 * Function to identify the index within the dksv struct of the last occurrence
 * of the specified char.
 * Returns -1 if no matching characters are found.
 */

DKSV_DEF int64_t dksv_idx_last(dksv sv, char c) {
  if (dksv_isempty(sv)) return -1;
  const char *found = (const char *)memrchr(sv.data, c, sv.len);
  return (found) ? found - sv.data : -1;
} /* dksv_idx_last() */

/*
 * Function to identify the character in a dksv struct at the given index.
 * Returns -1 if index is out of bounds.
 */

DKSV_DEF int32_t dksv_char_at(dksv sv, size_t index) {
  if ((index >= sv.len) || (dksv_isempty(sv))) return -1;
  return (unsigned char)sv.data[index];
} /* dksv_char_at() */

/*
 * Function to count the number of times the specified character occurs within the given
 * dksv struct.
 */

DKSV_DEF size_t dksv_count_char(dksv sv, char c) {
  if (dksv_isempty(sv)) return 0;
  size_t count = 0;

  for (size_t n = 0; n < sv.len; n++) {
    if (sv.data[n] == c) count++;
  }

  return count;
} /* dksv_count_char() */

/*
 * Function to convert a standard c string into a dksv struct with a specified length.
 */

DKSV_DEF dksv dksv_view_wlen(const char *string, size_t len) {
  if (!string) return DKSV_NULL;
  return (dksv){.data = string, .len = len};
} /* dksv_view_wlen() */

/*
 * Function to convert a given dksv struct into a double-precision floating-point value.
 * Returns NAN if it fails to convert or if invalid characters are present.
 */

DKSV_DEF double dksv_to_dbl(dksv sv) {
  sv = dksv_trim(sv);
  if (dksv_isempty(sv)) return NAN;

  if (dksv_ic_equals(sv, dksv_view("nan"))) return NAN;
  if ((dksv_ic_equals(sv, dksv_view("inf"))) || (dksv_ic_equals(sv, dksv_view("+inf")))) return INFINITY;
  if (dksv_ic_equals(sv, dksv_view("-inf"))) return -INFINITY;

  size_t index = 0;
  int sign = 1;

  switch (sv.data[index]) {
    case '-': sign = -1; /* fallthrough */
    case '+': index++; break;
    default:  break;
  }

  long double result = 0.0;
  bool has_digits = false;

  while ((index < sv.len) && (isdigit((unsigned char)sv.data[index]))) {
    result = (result * 10.0) + (sv.data[index] - '0');
    has_digits = true;
    index++;
  }

  if ((index < sv.len) && (sv.data[index] == '.')) {
    index++;
    long double fraction = 0.0;
    long double divisor = 1.0;

    while ((index < sv.len) && (isdigit((unsigned char)sv.data[index]))) {
      fraction = (fraction * 10.0) + (sv.data[index] - '0');
      divisor *= 10.0;
      has_digits = true;
      index++;
    }

    result += fraction / divisor;
  }

  if (!has_digits) return NAN;

  if ((index < sv.len) && (toupper((unsigned char)sv.data[index]) == 'E')) {
    index++;
    int exp_sign = 1;

    if (index < sv.len) {
      switch (sv.data[index]) {
        case '-': exp_sign = -1; /* fallthrough */
        case '+': index++; break;
        default:  break;
      }
    }

    int exponent = 0;
    while ((index < sv.len) && (isdigit((unsigned char)sv.data[index]))) {
      if (exponent < 9999) exponent = (exponent * 10) + (sv.data[index] - '0');
      index++;
    }

    if (exponent != 0) result *= pow(10.0, exp_sign * exponent);
  }

  return (index < sv.len) ? NAN : result * sign;
} /* dksv_to_dbl() */

#endif // DKSV_IMPLEMENTATION

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
