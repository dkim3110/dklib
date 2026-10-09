/*
# `dk_arena.h` - public domain - Daniel Inhoi Kim, 2026
Header-only library for virtual memory arenas. See end of file for license information.

## Usage
Macros that enable function definitions (excluding the MAKE_STATIC macros) must only be defined once in one source file
before including the header. See
https://github.com/nothings/stb/blob/f58f558c120e9b32c217290b80bad1a0729fbb2c/docs/stb_howto.txt for more information.

This library requires GCC or Clang with GNU C extensions, so compile with `-std=gnu11`. It targets Linux and Windows
only. Windows support is written against the Win32 API but untested.

This library does not concern itself with C++ support.

### Flags
- DKA_IMPLEMENTATION: enable function definitions
- DKA_DYNAMIC_ARRAY: enable dynamic array function declarations
- DKA_DYNAMIC_ARRAY_IMPLEMENTATION: enable dynamic array function definitions
- DKA_STRING_BUILDER: enable string function declarations
- DKA_STRING_BUILDER_IMPLEMENTATION: enable string function definitions
- DKA_GIMME_ALL: include all function definitions
- DKA_GIMME_ALL_DEC: include all declarations; no definitions
- DKA_IMPLEMENTATION_MAKE_STATIC: same as `DKA_IMPLEMENTATION` but all functions are static inline
- DKA_GIMME_ALL_MAKE_STATIC: same as `DKA_GIMME_ALL` but all functions are static inline

### Optional Function Parameters
Several function-like macros (`dka_init()`, `dka_alloc()`, `strb_remove()`, etc.) accept optional parameters. You can
set custom values:

  ```C
  char *buf = dka_alloc(arena, KiB(1), .align = 64);
  ```

Consult the parameter structs for a full list of defaults.

### OOM Handler Function
This library provides a default oom handler that exits the program with an error code. You can use your own handler if
you so choose. The oom handler should ideally exit the program.

  ```C
  dk_arena *arena = dka_init(.oom_handler = custom_function);
  ```

### Example Code
  ```C
  #define DKA_GIMME_ALL
  #include "dk_arena.h"

  #include <stdio.h>

  int main(void) {
    dk_arena *arena = dka_init(.name = "Main Arena"); // initialize arena
    if (!arena) return 1;

    strb text = strb_build(arena, "Hello, World"); // initialize string builder
    dka_temp tmp = dka_take_snapshot(arena); // begin temporary arena

    size_t *numbers = darr_init(arena, size_t, 10); // initialize dynamic array
    for (size_t n = 0; n < 20; n++) {
      darr_push(arena, numbers, n);
    }

    dka_drop_snapshot(tmp); // end temporary arena
    strb_append(arena, text, '!');
    printf("%s\n", text);

    dka_delete(arena); // deallocate all at once
    return 0;
  }
  ```
 */

#ifdef DKA_GIMME_ALL_MAKE_STATIC
  #define DKA_GIMME_ALL
  #define DKA_IMPLEMENTATION_MAKE_STATIC
#endif // DKA_GIMME_ALL_MAKE_STATIC

#ifdef DKA_GIMME_ALL
  #define DKA_IMPLEMENTATION
  #define DKA_DYNAMIC_ARRAY_IMPLEMENTATION
  #define DKA_STRING_BUILDER_IMPLEMENTATION
#endif // DKA_GIMME_ALL

#ifdef DKA_GIMME_ALL_DEC
  #define DKA_DYNAMIC_ARRAY
  #define DKA_STRING_BUILDER
#endif // DKA_GIMME_ALL_DEC

#ifdef DKA_IMPLEMENTATION_MAKE_STATIC
  #define DKA_DEC static inline
  #define DKA_DEF DKA_DEC

  #define DKA_IMPLEMENTATION

  #ifdef DKA_DYNAMIC_ARRAY
    #define DKA_DYNAMIC_ARRAY_IMPLEMENTATION
  #endif // DKA_DYNAMIC_ARRAY

  #ifdef DKA_STRING_BUILDER
    #define DKA_STRING_BUILDER_IMPLEMENTATION
  #endif // DKA_STRING_BUILDER
#else
  #define DKA_DEC extern
  #define DKA_DEF
#endif // DKA_IMPLEMENTATION_MAKE_STATIC

#ifdef DKA_STRING_BUILDER_IMPLEMENTATION
  #define DKA_STRING_BUILDER
  #define DKA_DYNAMIC_ARRAY
#endif // DKA_STRING_BUILDER_IMPLEMENTATION

#ifdef DKA_STRING_BUILDER
  #define DKA_DYNAMIC_ARRAY
#endif // DKA_STRING_BUILDER

#ifdef DKA_DYNAMIC_ARRAY_IMPLEMENTATION
  #define DKA_DYNAMIC_ARRAY
#endif // DKA_DYNAMIC_ARRAY_IMPLEMENTATION

#ifndef DK_ARENA_H_
#define DK_ARENA_H_

#ifdef __linux__
  #include <sys/mman.h>
  #include <unistd.h>
#elif defined(_WIN32)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif // WIN32_LEAN_AND_MEAN

  #include <windows.h>
#else
  #error "Unsupported Operating System"
#endif // OPERATING SYSTEM

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dk_arena {
  size_t reserve;                                // total virtual memory reserved
  size_t commit;                                 // virtual memory to commit at a time
  size_t pos;                                    // current pointer position
  size_t commit_pos;                             // current commit position
  void (*handle_oom)(struct dk_arena *, size_t); // function pointer to an oom error handler; ideally exits program
  const char *name;                              // optionally give your arena a name; must outlive arena
  uint32_t page_size;                            // system page size
} dk_arena;

typedef struct {
  dk_arena *arena; // pointer to parent arena
  size_t pos;      // position within parent arena
} dka_temp;

typedef struct {
  uint8_t dummy;                           // dummy field for initialization
  size_t reserve_size;                     // default: 1 GiB
  size_t commit_size;                      // default: 2 MiB
  void (*oom_handler)(dk_arena *, size_t); // default: dka_oom_handler()
  const char *name;                        // default: NULL
  size_t align;                            // default: DKA_ALIGN
} dka__params;

#define DKA_ALIGN_UP_POW2(n, p) (((size_t)(n) + ((size_t)(p) - 1)) & (~((size_t)(p) - 1)))
#define DKA_ALIGN_UP(n, a)      ((((size_t)(n) + ((size_t)(a) - 1)) / (size_t)(a)) * (size_t)(a))
#define DKA_BASE_POS            (sizeof(dk_arena))
#define DKA_ALIGN               (alignof(max_align_t))

#define KiB(n) ((size_t)(n) << 10)
#define MiB(n) ((size_t)(n) << 20)
#define GiB(n) ((size_t)(n) << 30)

#define dka_init(...) impl_dka__initialization((dka__params){.dummy = 0, ##__VA_ARGS__})

#define dka_alloc(arena, size, ...)                                                                                    \
  impl_dka__allocation((arena), (dka__params){.dummy = 0, ##__VA_ARGS__}, (size), false)

#define dka_calloc(arena, count, size, ...)                                                                            \
  impl_dka__callocation((arena), (dka__params){.dummy = 0, ##__VA_ARGS__}, (count), (size))

#define dka_realloc(arena, ptr, oldsz, newsz, ...)                                                                     \
  impl_dka__reallocation((arena), (dka__params){.dummy = 0, ##__VA_ARGS__}, (ptr), (oldsz), (newsz))

#define dka_clear(arena) impl_dka_resetting((arena), false)
#define dka_purge(arena) impl_dka_resetting((arena), true)

#define dka_take_snapshot(a)        ((dka_temp){.arena = (a), .pos = (a)->pos})
#define dka_drop_snapshot(dka_temp) dka_rewind((dka_temp).arena, (dka_temp).pos)

DKA_DEC void dka_oom_handler(dk_arena *, size_t);
DKA_DEC void dka_delete(dk_arena *);
DKA_DEC void dka_rewind(dk_arena *, size_t);

DKA_DEC dk_arena *impl_dka__initialization(dka__params);
DKA_DEC void *impl_dka__allocation(dk_arena *, dka__params, size_t, bool);
DKA_DEC void *impl_dka__callocation(dk_arena *, dka__params, size_t, size_t);
DKA_DEC void *impl_dka__reallocation(dk_arena *, dka__params, void *, size_t, size_t);
DKA_DEC void impl_dka__resetting(dk_arena *, bool);

DKA_DEC uint32_t impl_dka__sys_get_pagesize(void);
DKA_DEC void *impl_dka__sys_mem_reserve(size_t);
DKA_DEC bool impl_dka__sys_mem_commit(void *, size_t);
DKA_DEC bool impl_dka__sys_mem_decommit(void *, size_t);
DKA_DEC bool impl_dka__sys_mem_release(void *, size_t);

#endif // DK_ARENA_H_

#if defined(DKA_IMPLEMENTATION) && !defined(DKA_IMPLEMENTATION_GUARD)
#define DKA_IMPLEMENTATION_GUARD

DKA_DEF void dka_oom_handler(dk_arena *arena, size_t requested_size) {
  fprintf(stderr,
          "[dk_arena%s%s] OUT OF MEMORY\n"
          " | Attempted to allocate: %zu bytes\n"
          " | Remaining capacity   : %zu bytes\n",
          (arena->name) ? ": " : "",
          (arena->name) ? arena->name : "",
          requested_size,
          (arena->reserve - arena->pos));

  exit(EXIT_FAILURE);
} /* dka_oom_handler() */

DKA_DEF void dka_delete(dk_arena *arena) {
  if (arena) impl_dka__sys_mem_release(arena, arena->reserve);
} /* dka_delete() */

DKA_DEF void dka_rewind(dk_arena *arena, size_t pos) {
  if (!arena) return;
  size_t new_pos = (pos < DKA_BASE_POS) ? DKA_BASE_POS : pos;
  if (new_pos < arena->pos) {
    arena->pos = new_pos;
  }
} /* dka_rewind() */

DKA_DEF dk_arena *impl_dka__initialization(dka__params params) {
  if (!params.reserve_size) params.reserve_size = GiB(1);
  if (!params.commit_size) params.commit_size = MiB(2);
  if (!params.oom_handler) params.oom_handler = dka_oom_handler;

  if (params.commit_size > params.reserve_size) params.commit_size = params.reserve_size;

  uint32_t page_size = impl_dka__sys_get_pagesize();
  if ((!page_size) || ((page_size & (page_size - 1)) != 0)) return NULL;
  if (params.reserve_size > SIZE_MAX - DKA_BASE_POS - page_size) return NULL;

  params.reserve_size += DKA_BASE_POS;
  params.commit_size = (params.commit_size < page_size) ? page_size : params.commit_size;

  params.reserve_size = DKA_ALIGN_UP_POW2(params.reserve_size, page_size);
  params.commit_size = DKA_ALIGN_UP_POW2(params.commit_size, page_size);

  dk_arena *arena = impl_dka__sys_mem_reserve(params.reserve_size);
  if (!arena) return NULL;
  if (!impl_dka__sys_mem_commit(arena, page_size)) {
    impl_dka__sys_mem_release(arena, params.reserve_size);
    return NULL;
  }

  arena->page_size = page_size;
  arena->reserve = params.reserve_size;
  arena->commit = params.commit_size;
  arena->pos = DKA_BASE_POS;
  arena->commit_pos = page_size;
  arena->name = params.name;
  arena->handle_oom = params.oom_handler;
  return arena;
} /* impl_dka__initialization() */

DKA_DEF void *impl_dka__allocation(dk_arena *arena, dka__params params, size_t size, bool zero_out) {
  if (!arena) return NULL;

  if (!params.align) params.align = DKA_ALIGN;
  if ((params.align & (params.align - 1)) != 0) return NULL;

  uintptr_t base = (uintptr_t)arena;
  size_t pos_align = (size_t)(DKA_ALIGN_UP_POW2(base + arena->pos, params.align) - base);
  size_t new_pos = pos_align + size;

  if ((new_pos < pos_align) || (new_pos > arena->reserve)) goto error_oom;

  if (new_pos > arena->commit_pos) {
    size_t new_commit_pos = DKA_ALIGN_UP(new_pos, arena->commit);
    new_commit_pos = (new_commit_pos < arena->reserve) ? new_commit_pos : arena->reserve;

    uint8_t *mem = (uint8_t *)arena + arena->commit_pos;
    size_t commit_size = new_commit_pos - arena->commit_pos;

    if (!impl_dka__sys_mem_commit(mem, commit_size)) goto error_oom;
    arena->commit_pos = new_commit_pos;
  }

  arena->pos = new_pos;

  uint8_t *out = (uint8_t *)arena + pos_align;
  if (zero_out) memset(out, 0, size);

  return out;

error_oom:
  arena->handle_oom(arena, size);
  return NULL;
} /* impl_dka__allocation() */

DKA_DEF void *impl_dka__callocation(dk_arena *arena, dka__params params, size_t count, size_t size) {
  if (!arena) return NULL;
  if ((size) && (count > SIZE_MAX / size)) goto error_oom;
  return impl_dka__allocation(arena, params, count * size, true);

error_oom:
  arena->handle_oom(arena, SIZE_MAX);
  return NULL;
} /* impl_dka__callocation */

DKA_DEF void *impl_dka__reallocation(dk_arena *arena, dka__params params, void *ptr, size_t old_size, size_t new_size) {
  if (!arena) return NULL;
  if ((!ptr) || (!old_size)) return impl_dka__allocation(arena, params, new_size, false);

  uintptr_t p = (uintptr_t)ptr, b = (uintptr_t)arena;
  if (p < b + DKA_BASE_POS) return NULL;
  size_t ptr_offset = (size_t)(p - b);
  if (ptr_offset > arena->pos || old_size > arena->pos - ptr_offset) return NULL;

  bool is_most_recent = (ptr_offset + old_size == arena->pos);
  if (new_size <= old_size) {
    if (is_most_recent) arena->pos -= old_size - new_size;
    return ptr;
  }

  if (is_most_recent) {
    if (!impl_dka__allocation(arena, (dka__params){.align = 1}, new_size - old_size, false)) return NULL;
    return ptr;
  }

  void *new_ptr = impl_dka__allocation(arena, params, new_size, false);
  if (new_ptr) memcpy(new_ptr, ptr, old_size);
  return new_ptr;
} /* impl_dka__reallocation() */

DKA_DEF void impl_dka__resetting(dk_arena *arena, bool do_decommit) {
  if (!arena) return;

  if (do_decommit) {
    if (arena->commit_pos > arena->page_size) {
      size_t decommit_size = arena->commit_pos - arena->page_size;
      uint8_t *decommit_ptr = (uint8_t *)arena + arena->page_size;

      impl_dka__sys_mem_decommit(decommit_ptr, decommit_size);
      arena->commit_pos = arena->page_size;
    }
  }

  dka_rewind(arena, DKA_BASE_POS);
} /* impl_dka__resetting() */

DKA_DEF uint32_t impl_dka__sys_get_pagesize(void) {
#ifdef __linux__
  return (uint32_t)sysconf(_SC_PAGESIZE);
#elif defined(_WIN32)
  SYSTEM_INFO sys_info = {0};
  GetSystemInfo(&sys_info);
  return sys_info.dwPageSize;
#endif
} /* impl_dka__sys_get_pagesize() */

DKA_DEF void *impl_dka__sys_mem_reserve(size_t size) {
#ifdef __linux__
  void *out = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
  return (out == MAP_FAILED) ? NULL : out;
#elif defined(_WIN32)
  return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
#endif
} /* impl_dka__sys_mem_reserve() */

DKA_DEF bool impl_dka__sys_mem_commit(void *ptr, size_t size) {
#ifdef __linux__
  return (mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0);
#elif defined(_WIN32)
  return (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE));
#endif
} /* impl_dka__sys_mem_commit() */

DKA_DEF bool impl_dka__sys_mem_decommit(void *ptr, size_t size) {
#ifdef __linux__
  if (mprotect(ptr, size, PROT_NONE) != 0) return false;
  return (madvise(ptr, size, MADV_DONTNEED) == 0);
#elif defined(_WIN32)
  return VirtualFree(ptr, size, MEM_DECOMMIT);
#endif
} /* impl_dka__sys_mem_decommit() */

DKA_DEF bool impl_dka__sys_mem_release(void *ptr, size_t size) {
#ifdef __linux__
  return (munmap(ptr, size) == 0);
#elif defined(_WIN32)
  (void)size;
  return VirtualFree(ptr, 0, MEM_RELEASE);
#endif
} /* impl_dka__sys_mem_release() */

#endif // DKA_IMPLEMENTATION

#if defined(DKA_STRING_BUILDER) && !defined(DKA_STRING_BUILDER_GUARD)
#define DKA_STRING_BUILDER_GUARD

#include <ctype.h>

typedef char *strb; // string builder

typedef struct {
  uint8_t dummy;   // dummy field for initialization
  bool ignore_ord; // default: false
} strb__params;

#define DKA_IS_PTR(x)   _Generic(&(x), char **: true, const char **: true, default: false)
#define DKA_STRLEN(str) (DKA_IS_PTR(str) ? strlen(str) : (sizeof(str) - 1))

#define strb_build(arena, str)           impl_strb__build((arena), (str), DKA_STRLEN(str))
#define strb_strdup(arena, sb)           impl_strb__build((arena), (sb), strb_strlen(sb));
#define strb_null_terminate(arena, sb)   impl_strb__push_null((arena), &(sb))
#define strb_tolower(sb)                 impl_strb__letter_case_shift(&(sb), true)
#define strb_toupper(sb)                 impl_strb__letter_case_shift(&(sb), false)
#define strb_insert(arena, sb, c, n)     impl_strb__insertion((arena), &(sb), (c), (n))
#define strb_append(arena, sb, c)        impl_strb__insertion((arena), &(sb), (c), strb_strlen(sb))
#define strb_concat(arena, sb, str, len) impl_strb__concatenation((arena), &(sb), (str), (len))
#define strb_remove(sb, n, ...)          impl_strb__removal(&(sb), (n), (strb__params){.dummy = 0, ##__VA_ARGS__})
#define strb_reverse(sb)                 impl_strb__reversal(&(sb))

DKA_DEC size_t strb_strlen(strb);
DKA_DEC bool strb_isempty(strb);
DKA_DEC bool strb_equals(strb, strb);
DKA_DEC bool strb_equals_ic(strb, strb);
DKA_DEC int strb_compare(strb, strb);

DKA_DEC strb impl_strb__build(dk_arena *, const char *, size_t len);
DKA_DEC bool impl_strb__push_null(dk_arena *, strb *);
DKA_DEC bool impl_strb__pop_null(strb *);
DKA_DEC bool impl_strb__letter_case_shift(strb *, bool);
DKA_DEC bool impl_strb__insertion(dk_arena *, strb *, char, size_t);
DKA_DEC bool impl_strb__concatenation(dk_arena *, strb *, char *, size_t);
DKA_DEC bool impl_strb__removal(strb *, size_t, strb__params);
DKA_DEC bool impl_strb__reversal(strb *);

#endif // DKA_STRING_BUILDER

#if defined(DKA_DYNAMIC_ARRAY) && !defined(DKA_DYNAMIC_ARRAY_GUARD)
#define DKA_DYNAMIC_ARRAY_GUARD

typedef union {
  struct {
    size_t count;
    size_t cap;
  } data;
  max_align_t align;
} darr_header;

#define DYNAMIC_ARRAY_INIT_CAPACITY (8)

#define darr_init(arena, type, size) impl_darr__initialization((arena), sizeof(type), (size))
#define darr_len(arr)                ((arr) ? ((darr_header *)(arr) - 1)->data.count : 0)
#define darr_cap(arr)                ((arr) ? ((darr_header *)(arr) - 1)->data.cap : 0)
#define darr_last(arr)               ((arr)[darr_len(arr) - 1])
#define darr_pop(arr)                (void)(((arr) && (darr_len(arr))) ? --((darr_header *)(arr) - 1)->data.count : 0)

#define darr_rev(arr)                                                                                                  \
  ({                                                                                                                   \
  bool darr_rev__success = false;                                                                                      \
  if (arr) {                                                                                                           \
    size_t darr_rev__len = darr_len(arr);                                                                              \
    if (darr_rev__len > 1) {                                                                                           \
      darr_rev__success = true;                                                                                        \
      __typeof__(arr) darr_rev__left = (arr);                                                                          \
      __typeof__(arr) darr_rev__right = darr_rev__left + (darr_rev__len - 1);                                          \
      for (; darr_rev__left < darr_rev__right; darr_rev__left++, darr_rev__right--) {                                  \
        __typeof__(*(arr)) darr_rev__tmp = (*darr_rev__left);                                                          \
        (*darr_rev__left) = (*darr_rev__right);                                                                        \
        (*darr_rev__right) = darr_rev__tmp;                                                                            \
      }                                                                                                                \
    }                                                                                                                  \
  }                                                                                                                    \
  darr_rev__success;                                                                                                   \
  })

#define darr_rem(arr, n)                                                                                               \
  ({                                                                                                                   \
  bool darr_rem__success = false;                                                                                      \
  if (arr) {                                                                                                           \
    size_t darr_rem__len = darr_len(arr);                                                                              \
    if ((n) < darr_rem__len) {                                                                                         \
      (arr)[(n)] = (arr)[darr_rem__len - 1];                                                                           \
      darr_pop(arr);                                                                                                   \
      darr_rem__success = true;                                                                                        \
    }                                                                                                                  \
  }                                                                                                                    \
  darr_rem__success;                                                                                                   \
  })

#define darr_rem_ord(arr, n)                                                                                           \
  ({                                                                                                                   \
  bool darr_rem_ord__success = false;                                                                                  \
  if (arr) {                                                                                                           \
    size_t darr_rem_ord__len = darr_len(arr);                                                                          \
    if ((n) < darr_rem_ord__len) {                                                                                     \
      memmove(&(arr)[(n)], &(arr)[(n) + 1], (darr_rem_ord__len - (n) - 1) * sizeof(*(arr)));                           \
      darr_pop(arr);                                                                                                   \
      darr_rem_ord__success = true;                                                                                    \
    }                                                                                                                  \
  }                                                                                                                    \
  darr_rem_ord__success;                                                                                               \
  })

#define darr_push(arena, arr, element)                                                                                 \
  ({                                                                                                                   \
  bool darr_push__success = true;                                                                                      \
  if (!(arr) || (darr_len(arr) >= darr_cap(arr))) {                                                                    \
    void *darr_push__tmp = impl_darr__growth((arena), (arr), sizeof(*(arr)));                                          \
    if (darr_push__tmp) (arr) = darr_push__tmp;                                                                        \
    else darr_push__success = false;                                                                                   \
  }                                                                                                                    \
  if (darr_push__success) (arr)[((darr_header *)(arr) - 1)->data.count++] = (element);                                 \
  darr_push__success;                                                                                                  \
  })

#define darr_reserve(arena, arr, size)                                                                                 \
  ({                                                                                                                   \
  bool darr_reserve__success = false;                                                                                  \
  if (arr) {                                                                                                           \
    void *darr_reserve__tmp = impl_darr__reservation((arena), (arr), sizeof(*(arr)), (size));                          \
    if (darr_reserve__tmp) {                                                                                           \
      (arr) = darr_reserve__tmp;                                                                                       \
      darr_reserve__success = true;                                                                                    \
    }                                                                                                                  \
  }                                                                                                                    \
  darr_reserve__success;                                                                                               \
  })

DKA_DEC void *impl_darr__initialization(dk_arena *, size_t, size_t);
DKA_DEC void *impl_darr__growth(dk_arena *, void *, size_t);
DKA_DEC void *impl_darr__reservation(dk_arena *, void *, size_t, size_t);

#endif // DKA_DYNAMIC_ARRAY

#if defined(DKA_STRING_BUILDER_IMPLEMENTATION) && !defined(DKA_STRING_BUILDER_IMPLEMENTATION_GUARD)
#define DKA_STRING_BUILDER_IMPLEMENTATION_GUARD

DKA_DEF size_t strb_strlen(strb sb) {
  if (!sb) return 0;
  size_t len = darr_len(sb);
  if (!len) return 0;
  if (darr_last(sb) == '\0') len--;

  return len;
} /* strb_strlen() */

DKA_DEF bool strb_isempty(strb sb) {
  return ((!sb) || (strb_strlen(sb) == 0));
} /* strb_isempty() */

DKA_DEF bool strb_equals(strb a, strb b) {
  size_t a_len = strb_strlen(a);
  size_t b_len = strb_strlen(b);

  if (a_len != b_len) return false;
  else if (strb_isempty(a)) return true;

  return (memcmp(a, b, a_len) == 0);
} /* strb_equals() */

DKA_DEF bool strb_equals_ic(strb a, strb b) {
  size_t a_len = strb_strlen(a);
  size_t b_len = strb_strlen(b);

  if (a_len != b_len) return false;
  else if (strb_isempty(a)) return true;

  for (size_t n = 0; n < a_len; n++) {
    if (toupper((unsigned char)a[n]) != toupper((unsigned char)b[n])) return false;
  }

  return true;
} /* strb_equals_ic() */

DKA_DEF int strb_compare(strb a, strb b) {
  size_t a_len = strb_strlen(a);
  size_t b_len = strb_strlen(b);

  size_t cmp_len = (a_len < b_len) ? a_len : b_len;
  int cmp = (cmp_len) ? memcmp(a, b, cmp_len) : 0;

  if (cmp) return cmp;
  if (a_len != b_len) return (a_len < b_len) ? -1 : 1;
  return 0;
} /* strb_compare() */

DKA_DEF strb impl_strb__build(dk_arena *arena, const char *str, size_t len) {
  if ((!arena) || (!str)) return NULL;
  strb sb = impl_darr__initialization(arena, sizeof(char), len + 1);
  if (!sb) return NULL;

  memcpy(sb, str, len + 1);
  ((darr_header *)sb - 1)->data.count = len + 1;
  impl_strb__push_null(arena, &sb);
  return sb;
} /* impl_strb__build() */

DKA_DEF bool impl_strb__push_null(dk_arena *arena, strb *sb) {
  if (!sb) return false;
  if (!(*sb)) return false;

  size_t len = darr_len(*sb);
  if (!len) return false;
  if (darr_last(*sb) != '\0') return darr_push(arena, (*sb), '\0');

  return true;
} /* impl_strb__push_null() */

DKA_DEF bool impl_strb__pop_null(strb *sb) {
  if (!sb) return false;
  if (!(*sb)) return false;

  size_t len = darr_len(*sb);
  if (!len) return false;
  if (darr_last(*sb) == '\0') darr_pop(*sb);

  return true;
} /* impl_strb__pop_null() */

DKA_DEF bool impl_strb__letter_case_shift(strb *sb, bool to_lower) {
  if (!sb) return false;
  if (strb_isempty(*sb)) return false;

  for (size_t n = 0; n < strb_strlen(*sb); n++) {
    if (to_lower) (*sb)[n] = tolower((unsigned char)(*sb)[n]);
    else (*sb)[n] = toupper((unsigned char)(*sb)[n]);
  }

  return true;
} /* impl_strb__letter_case_shift() */

DKA_DEF bool impl_strb__insertion(dk_arena *arena, strb *sb, char c, size_t n) {
  if (!sb) return false;
  if (!(*sb)) return false;
  impl_strb__pop_null(sb);

  size_t len = strb_strlen(*sb);
  size_t cap = darr_cap(*sb);
  if (n > len) n = len;

  if (len >= ((!cap) ? 0 : cap - 1)) {
    char *tmp = impl_darr__growth(arena, (*sb), sizeof(char));
    if (!tmp) {
      impl_strb__push_null(arena, sb);
      return false;
    }
    (*sb) = tmp;
  }

  if (n < len) {
    size_t bytes_to_move = len - n;
    memmove((*sb) + n + 1, (*sb) + n, bytes_to_move);
  }

  (*sb)[n] = c;
  ((darr_header *)(*sb) - 1)->data.count++;

  return impl_strb__push_null(arena, sb);
} /* impl_strb__insertion() */

DKA_DEF bool impl_strb__concatenation(dk_arena *arena, strb *a, char *b, size_t b_len) {
  if ((!a) || (!b)) return false;
  if (!(*a)) return false;
  if (strb_isempty(b)) return true;

  size_t a_len = strb_strlen(*a);

  if (SIZE_MAX - a_len - 1 < b_len) return false;

  size_t a_cap = darr_cap(*a);
  impl_strb__pop_null(a);
  size_t requested_size = a_len + b_len + 1;

  if (requested_size > a_cap) {
    size_t new_cap = requested_size > a_cap * 2 ? requested_size : a_cap * 2;
    char *tmp = impl_darr__reservation(arena, (*a), sizeof(char), new_cap);
    if (!tmp) {
      impl_strb__push_null(arena, a);
      return false;
    }
    (*a) = tmp;
  }

  memmove((*a) + a_len, b, b_len);
  ((darr_header *)(*a) - 1)->data.count += b_len;

  return impl_strb__push_null(arena, a);
} /* impl_strb__concatenation() */

DKA_DEF bool impl_strb__removal(strb *sb, size_t n, strb__params params) {
  if (!sb) return false;
  if (strb_isempty(*sb)) return false;

  size_t len = strb_strlen(*sb);
  if (n >= len) return false;

  if (len == 1) {
    (*sb)[0] = '\0';
    ((darr_header *)(*sb) - 1)->data.count = 1;
    return true;
  }

  impl_strb__pop_null(sb);

  if (params.ignore_ord) {
    if (!darr_rem((*sb), n)) return false;
  } else {
    if (!darr_rem_ord((*sb), n)) return false;
  }

  (*sb)[len - 1] = '\0';
  ((darr_header *)(*sb) - 1)->data.count++;
  return true;
} /* impl_strb__removal() */

DKA_DEF bool impl_strb__reversal(strb *sb) {
  if (!sb) return false;
  if (strb_isempty(*sb)) return false;

  size_t len = strb_strlen(*sb);
  if (len <= 1) return true;

  char *left = (*sb);
  char *right = left + (len - 1);

  for (; left < right; left++, right--) {
    char tmp = (*left);
    (*left) = (*right);
    (*right) = tmp;
  }

  return true;
} /* impl_strb__reversal() */

#endif // DKA_STRING_BUILDER_IMPLEMENTATION

#if defined(DKA_DYNAMIC_ARRAY_IMPLEMENTATION) && !defined(DKA_DYNAMIC_ARRAY_IMPLEMENTATION_GUARD)
#define DKA_DYNAMIC_ARRAY_IMPLEMENTATION_GUARD

DKA_DEF void *impl_darr__initialization(dk_arena *arena, size_t elem_size, size_t count) {
  if (!elem_size) return NULL;
  if (count > (SIZE_MAX - sizeof(darr_header)) / elem_size) return NULL;

  darr_header *header = dka_alloc(arena, sizeof(darr_header) + (elem_size * count));
  if (!header) return NULL;

  header->data.count = 0;
  header->data.cap = count;

  return header + 1;
} /* impl_darr__initialization() */

DKA_DEF void *impl_darr__growth(dk_arena *arena, void *arr, size_t elem_size) {
  if (!elem_size) return NULL;
  if (!arr) return impl_darr__initialization(arena, elem_size, DYNAMIC_ARRAY_INIT_CAPACITY);

  darr_header *header = (darr_header *)arr - 1;
  if (header->data.cap > SIZE_MAX / 2) return NULL;
  size_t new_cap = (header->data.cap) ? header->data.cap * 2 : DYNAMIC_ARRAY_INIT_CAPACITY;

  return impl_darr__reservation(arena, arr, elem_size, new_cap);
} /* impl_darr__growth() */

DKA_DEF void *impl_darr__reservation(dk_arena *arena, void *arr, size_t elem_size, size_t size) {
  if ((!size) || (!elem_size)) return NULL;
  if (!arr) return impl_darr__initialization(arena, elem_size, size);

  darr_header *header = (darr_header *)arr - 1;
  if (header->data.cap >= size) return arr;
  if (size > (SIZE_MAX - sizeof(darr_header)) / elem_size) return NULL;

  size_t old_size = sizeof(darr_header) + (elem_size * header->data.cap);
  size_t new_size = sizeof(darr_header) + (elem_size * size);

  darr_header *new_header = dka_realloc(arena, header, old_size, new_size);
  if (!new_header) return NULL;

  header = new_header;
  header->data.cap = size;

  return header + 1;
} /* impl_darr__reservation() */

#endif // DKA_DYNAMIC_ARRAY_IMPLEMENTATION

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
