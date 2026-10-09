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
- DKSV_IMPLEMENTATION_MAKE_STATIC: enable dynamic array function declarations

### Example Code
  ```C
  #define DKSV_IMPLEMENTATION
  #include "dk_strview.h"

  #include <stdio.h>

  int main(void) {
    strv text = to_strv("  Hello, World!  ");
    print_strv(text);
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *data;
  size_t len;
} strv;

#define DKSV_NULL    ((strv){.data = NULL, .len = 0})
#define DKSV_FMT     "%.*s"
#define DKSV_ARG(sv) (int)(sv).len, (sv).data

#define DKSV_IS_PTR(x)   _Generic(&(x), char **: true, const char **: true, default: false)
#define DKSV_STRLEN(str) (DKSV_IS_PTR(str) ? strlen(str) : (sizeof(str) - 1))
#define to_strv(cstr)    to_strv_wlen((cstr), DKSV_STRLEN(cstr))

#ifdef DKA_STRING_BUILDER
  #define sb_to_sv(sb) to_strv_wlen((sb), strb_strlen(sb))
#endif // DKA_STRING_BUILDER

DKSV_DEC bool strv_isempty(strv);
DKSV_DEC bool strv_isblank(strv);
DKSV_DEC bool strv_equals(strv, strv);
DKSV_DEC bool strv_ic_equals(strv, strv);
DKSV_DEC int strv_cmp(strv, strv);
DKSV_DEC bool strv_isalpha(strv);
DKSV_DEC bool strv_isalnum(strv);
DKSV_DEC bool strv_isdigit(strv);

DKSV_DEC void fput_strv(strv, FILE *);
DKSV_DEC void print_strv(strv);
DKSV_DEC void println_strv(strv);

DKSV_DEC strv strv_take(strv, size_t);
DKSV_DEC strv strv_drop(strv, size_t);
DKSV_DEC strv strv_sub(strv, size_t, size_t);
DKSV_DEC strv strv_split(strv *, char);
DKSV_DEC strv strv_split_ws(strv *);

DKSV_DEC strv strv_trim(strv);
DKSV_DEC strv strv_ltrim(strv);
DKSV_DEC strv strv_rtrim(strv);
DKSV_DEC strv strv_trimc(strv, char);
DKSV_DEC strv strv_ltrimc(strv, char);
DKSV_DEC strv strv_rtrimc(strv, char);

DKSV_DEC bool strv_prefix(strv, strv);
DKSV_DEC bool strv_suffix(strv, strv);
DKSV_DEC bool strv_ic_prefix(strv, strv);
DKSV_DEC bool strv_ic_suffix(strv, strv);
DKSV_DEC bool strv_contains(strv, strv);
DKSV_DEC int64_t idx_strv_contains(strv, strv);
DKSV_DEC int64_t strv_idx(strv, char);
DKSV_DEC int64_t strv_idx_last(strv, char);
DKSV_DEC int32_t strv_char_at(strv, size_t);
DKSV_DEC size_t strv_count_char(strv, char);

DKSV_DEC strv to_strv_wlen(const char *, size_t);
DKSV_DEC double strv_to_dbl(strv);

#endif // DK_STR_VIEW_H_

#if defined(DKSV_IMPLEMENTATION) && !defined(DKSV_IMPLEMENTATION_GUARD)
#define DKSV_IMPLEMENTATION_GUARD

#include <ctype.h>
#include <math.h>

/*
 * Function to verify if a given strv struct is empty.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool strv_isempty(strv sv) {
  return ((sv.len == 0) || (!sv.data));
} /* strv_isempty() */

/*
 * Function to verify if a given strv struct is blank: either emoty or consisting of only whitespaces.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool strv_isblank(strv sv) {
  if (strv_isempty(sv)) return true;

  for (size_t n = 0; n < sv.len; n++) {
    if (!isspace((unsigned char)sv.data[n])) return false;
  }

  return true;
} /* strv_isblank() */

/*
 * Function to verify if two strv structs are equal.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool strv_equals(strv a, strv b) {
  if (a.len != b.len) return false;
  if (strv_isempty(a) || strv_isempty(b)) return (strv_isempty(a) == strv_isempty(b));
  return (memcmp(a.data, b.data, a.len) == 0);
} /* strv_equals() */

/*
 * Function to verify if two strv structs are equal, ignoring letter case.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool strv_ic_equals(strv a, strv b) {
  if (a.len != b.len) return false;
  if (strv_isempty(a) || strv_isempty(b)) return (strv_isempty(a) == strv_isempty(b));

  for (size_t n = 0; n < a.len; n++) {
    if (toupper((unsigned char)a.data[n]) != toupper((unsigned char)b.data[n])) return false;
  }

  return true;
} /* strv_ic_equals() */

/*
 * Function to compare two strings.
 * Returns 0 if a and b are equal, a negative value if a is less than b,
 * and a positive value if a is greater than b.
 */

DKSV_DEF int strv_cmp(strv a, strv b) {
  size_t cmp_len = (a.len < b.len) ? a.len : b.len;
  int cmp = (cmp_len) ? memcmp(a.data, b.data, cmp_len) : 0;

  if (cmp) return cmp;
  if (a.len != b.len) return (a.len < b.len) ? -1 : 1;
  return 0;
} /* strv_cmp() */

/*
 * Function to determine if a given strv struct consists of only alphabet characters.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool strv_isalpha(strv sv) {
  if (strv_isempty(sv)) return false;

  for (size_t n = 0; n < sv.len; n++) {
    if (!isalpha((unsigned char)sv.data[n])) return false;
  }

  return true;
} /* strv_isalpha() */

/*
 * Function to determine if a given strv struct consists of only alphabet characters
 * and digits.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool strv_isalnum(strv sv) {
  if (strv_isempty(sv)) return false;

  for (size_t n = 0; n < sv.len; n++) {
    if (!isalnum((unsigned char)sv.data[n])) return false;
  }

  return true;
} /* strv_isalnum() */

/*
 * Function to determine if a given strv struct consists of only digits.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool strv_isdigit(strv sv) {
  if (strv_isempty(sv)) return false;

  for (size_t n = 0; n < sv.len; n++) {
    if (!isdigit((unsigned char)sv.data[n])) return false;
  }

  return true;
} /* strv_isdigit() */

/*
 * Function to write a strv struct to a specified file stream.
 */

DKSV_DEF void fput_strv(strv sv, FILE *stream) {
  if ((!stream) || (strv_isempty(sv))) return;
  fwrite(sv.data, sizeof(char), sv.len, stream);
} /* fput_strv() */

/*
 * Function to print a strv struct to stdout.
 */

DKSV_DEF void print_strv(strv sv) {
  fput_strv(sv, stdout);
} /* print_strv() */

/*
 * Function to print a strv struct to stdout, then a newline character.
 */

DKSV_DEF void println_strv(strv sv) {
  print_strv(sv);
  putchar('\n');
} /* println_strv() */

/*
 * Function to take characters from a given strv struct up to the specified index.
 * It is exclusive and thus will not take the character at index n.
 * Returns sv unmodified if empty.
 */

DKSV_DEF strv strv_take(strv sv, size_t n) {
  if (strv_isempty(sv)) return sv;
  if (n > sv.len) n = sv.len;

  return (strv){.data = sv.data, .len = n};
} /* strv_take() */

/*
 * Function to drop characters from a strv struct up to the specified index.
 * It is exclusive and thus will not drop the character at index n.
 * Returns sv unmodifed if empty.
 */

DKSV_DEF strv strv_drop(strv sv, size_t n) {
  if (strv_isempty(sv)) return sv;
  if (n > sv.len) n = sv.len;

  return (strv){.data = sv.data + n, .len = sv.len - n};
} /* strv_drop() */

/*
 * Function to extract a substring from a strv struct from the start and end indexes provided.
 * Start is inclusive, end is exclusive.
 */

DKSV_DEF strv strv_sub(strv sv, size_t start, size_t end) {
  return strv_drop(strv_take(sv, end), start);
} /* strv_sub() */

/*
 * Function to split a given strv struct based on the specifed delimiter.
 * Returns a strv struct up to the first instance of the specified delimiter, or the original string
 * if it is empty. The original strv struct retains all characters after the
 * delimiter; mutated by the function. The delimiter itself gets consumed.
 */

DKSV_DEF strv strv_split(strv *sv, char delimiter) {
  if (strv_isempty(*sv)) return *sv;

  const char *found = (const char *)memchr(sv->data, delimiter, sv->len);
  size_t n = (found) ? (size_t)(found - sv->data) : sv->len;

  strv result = strv_take(*sv, n);
  if (n < sv->len) *sv = strv_drop(*sv, n + 1);
  else *sv = strv_drop(*sv, n);

  return result;
} /* strv_split() */

/*
 * Function to split a given strv struct by whitespaces.
 * Returns a strv struct up to the first instance of a whitespace, or the original string
 * if it is empty. The original strv struct retains all characters after the
 * delimiter; mutated by the function. The delimiter itself gets consumed.
 */

DKSV_DEF strv strv_split_ws(strv *sv) {
  if (strv_isempty(*sv)) return *sv;

  size_t n = 0;
  while ((n < sv->len) && (!isspace(sv->data[n]))) n++;

  strv result = strv_take(*sv, n);
  if (n < sv->len) *sv = strv_drop(*sv, n + 1);
  else *sv = strv_drop(*sv, n);

  return result;
} /* strv_split() */

/*
 * Function to trim leading and trailing whitespace from a given strv struct.
 * Returns original string if the given string is empty.
 */

DKSV_DEF strv strv_trim(strv sv) {
  return strv_ltrim(strv_rtrim(sv));
} /* strv_trim() */

/*
 * Function to trim leading whitespace from a given strv struct.
 * Returns original string if the given string is empty.
 *
 */

DKSV_DEF strv strv_ltrim(strv sv) {
  if (strv_isempty(sv)) return sv;
  while ((sv.len > 0) && (isspace((unsigned char)sv.data[0]))) sv = strv_drop(sv, 1);

  return sv;
} /* strv_ltrim() */

/*
 * Function to trim trailing whitespace from a given strv struct.
 * Returns original string if the given string is empty.
 */

DKSV_DEF strv strv_rtrim(strv sv) {
  if (strv_isempty(sv)) return sv;
  while ((sv.len > 0) && (isspace((unsigned char)sv.data[sv.len - 1]))) sv = strv_take(sv, sv.len - 1);

  return sv;
} /* strv_rtrim() */

/*
 * Function to trim leading and trailing instances of the provided character from
 * a given strv struct.
 * Returns original string if the given string is empty.
 */

DKSV_DEF strv strv_trimc(strv sv, char c) {
  return strv_ltrimc(strv_rtrimc(sv, c), c);
} /* strv_trimc() */

/*
 * Function to trim leading instances of the provided character from a given strv struct.
 * Returns original string if the given string is empty.
 */

DKSV_DEF strv strv_ltrimc(strv sv, char c) {
  if (strv_isempty(sv)) return sv;
  while ((sv.len > 0) && (sv.data[0] == c)) sv = strv_drop(sv, 1);

  return sv;
} /* strv_ltrimc() */

/*
 * Function to trim trailing instances of the provided character from a given strv struct.
 * Returns original string if the given string is empty.
 */

DKSV_DEF strv strv_rtrimc(strv sv, char c) {
  if (strv_isempty(sv)) return sv;
  while ((sv.len > 0) && (sv.data[sv.len - 1] == c)) sv = strv_take(sv, sv.len - 1);

  return sv;
} /* strv_rtrimc() */

/*
 * Function to verify if a given strv struct starts with the specified prefix.
 * Returns 1 if true, 0 if false. Empty strings are considered to be contained by
 * all strings.
 */

DKSV_DEF bool strv_prefix(strv base, strv prefix) {
  if (strv_isempty(prefix)) return true;
  if ((!base.data) || (!prefix.data)) return false;
  if ((prefix.len > base.len) || (strv_isempty(base))) return false;
  return (memcmp(base.data, prefix.data, prefix.len) == 0);
} /* strv_prefix() */

/*
 * Function to verify if a given strv struct ends with the specified suffix.
 * Returns 1 if true, 0 if false. Empty strings are considered to be contained by
 * all strings.
 */

DKSV_DEF bool strv_suffix(strv base, strv suffix) {
  if (strv_isempty(suffix)) return true;
  if ((!base.data) || (!suffix.data)) return false;
  if ((suffix.len > base.len) || (strv_isempty(base))) return false;
  return (memcmp(base.data + (base.len - suffix.len), suffix.data, suffix.len) == 0);
} /* strv_suffix() */

/*
 * Function to verify if a given strv struct starts with the specified prefix, ignoring letter case.
 * Returns 1 if true, 0 if false. Empty strings are considered to be contained by
 * all strings.
 */

DKSV_DEF bool strv_ic_prefix(strv base, strv prefix) {
  if (strv_isempty(prefix)) return true;
  if ((!base.data) || (!prefix.data)) return false;
  if ((prefix.len > base.len) || (strv_isempty(base))) return false;
  return strv_ic_equals(strv_take(base, prefix.len), prefix);
} /* strv_ic_prefix() */

/*
 * Function to verify if a given strv struct ends with the specified suffix, ignoring letter case.
 * Returns 1 if true, 0 if false. Empty strings are considered to be contained by
 * all strings.
 */

DKSV_DEF bool strv_ic_suffix(strv base, strv suffix) {
  if (strv_isempty(suffix)) return true;
  if ((!base.data) || (!suffix.data)) return false;
  if ((suffix.len > base.len) || (strv_isempty(base))) return false;
  return strv_ic_equals(strv_drop(base, base.len - suffix.len), suffix);
} /* strv_ic_suffix() */

/*
 * Function to verify if a given strv struct contains the specified string.
 * Empty strings are considered to be contained by all strings.
 * Returns 1 if true, 0 if false.
 */

DKSV_DEF bool strv_contains(strv base, strv item) {
  return (idx_strv_contains(base, item) != -1);
} /* strv_contains() */

/*
 * Function to verify if a given strv struct contains the specified string.
 * Empty strings are considered to be contained by all strings. Returns the index
 * of the first occurrence or -1 if the base strv does not contain the item.
 */

DKSV_DEF int64_t idx_strv_contains(strv base, strv item) {
  if (strv_isempty(item)) return 0;
  if ((item.len > base.len) || (strv_isempty(base))) return -1;

  char *sub = memmem(base.data, base.len, item.data, item.len);
  return (sub) ? sub - base.data : -1;
} /* idx_strv_contains() */

/*
 * Function to identify the index within the strv struct of the first occurrence
 * of the specified char.
 * Returns -1 if no matching characters are found.
 */

DKSV_DEF int64_t strv_idx(strv sv, char c) {
  if (strv_isempty(sv)) return -1;
  const char *found = (const char *)memchr(sv.data, c, sv.len);
  return (found) ? found - sv.data : -1;
} /* strv_idx() */

/*
 * Function to identify the index within the strv struct of the last occurrence
 * of the specified char.
 * Returns -1 if no matching characters are found.
 */

DKSV_DEF int64_t strv_idx_last(strv sv, char c) {
  if (strv_isempty(sv)) return -1;
  const char *found = (const char *)memrchr(sv.data, c, sv.len);
  return (found) ? found - sv.data : -1;
} /* strv_idx_last() */

/*
 * Function to identify the character in a strv struct at the given index.
 * Returns -1 if index is out of bounds.
 */

DKSV_DEF int32_t strv_char_at(strv sv, size_t index) {
  if ((index >= sv.len) || (strv_isempty(sv))) return -1;
  return (unsigned char)sv.data[index];
} /* strv_char_at() */

/*
 * Function to count the number of times the specified character occurs within the given
 * strv struct.
 */

DKSV_DEF size_t strv_count_char(strv sv, char c) {
  if (strv_isempty(sv)) return 0;
  size_t count = 0;

  for (size_t n = 0; n < sv.len; n++) {
    if (sv.data[n] == c) count++;
  }

  return count;
} /* strv_count_char() */

/*
 * Function to convert a standard c string into a strv struct with a specified length.
 */

DKSV_DEF strv to_strv_wlen(const char *cstring, size_t len) {
  if (!cstring) return DKSV_NULL;
  return (strv){.data = cstring, .len = len};
} /* to_strv() */

/*
 * Function to convert a given strv struct into a double-precision floating-point value.
 * Returns NAN if it fails to convert or if invalid characters are present.
 */

DKSV_DEF double strv_to_dbl(strv sv) {
  sv = strv_trim(sv);
  if (strv_isempty(sv)) return NAN;

  if (strv_ic_equals(sv, to_strv("nan"))) return NAN;
  if ((strv_ic_equals(sv, to_strv("inf"))) || (strv_ic_equals(sv, to_strv("+inf")))) return INFINITY;
  if (strv_ic_equals(sv, to_strv("-inf"))) return -INFINITY;

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
} /* strv_to_dbl() */

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
