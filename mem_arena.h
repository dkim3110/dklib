// mem_arena.h - public domain - Daniel Inhoi Kim, 2026
// Header-only library for virtual memory arenas.
// See end of file for license information.

/* flags */
// - MEM_ARENA_IMPLEMENTATION: enable function definitions
//
// - MEM_ARENA_DYNAMIC_ARRAY: enable dynamic array function declarations
// - MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION: enable dynamic array function definitions
// - MEM_ARENA_GIMME_DYNAMIC_ARRAY: enable dynamic array function declarations + definitions
//
// - MEM_ARENA_STRING_BUILDER: enable string function declarations
// - MEM_ARENA_STRING_BUILDER_IMPLEMENTATION: enable string function definitions
// - MEM_ARENA_GIMME_STRING_BUILDER: enable string function declarations + definitions
//
// - MEM_ARENA_GIMME_ALL: define all of the above
// - MEM_ARENA_GIMME_ALL_DEC: include all declarations; no definitions
//
// - MEM_ARENA_MAKE_STATIC: make all functions static inline + MEM_ARENA_IMPLEMENTATION
// - MEM_ARENA_GIMME_ALL_MAKE_STATIC: same as MEM_ARENA_GIMME_ALL but all functions are static inline

/*
 * Macros that enable function definitions (excluding the MAKE_STATIC macros) must only be defined once in one source
 * file before including the header. See
 * https://github.com/nothings/stb/blob/f58f558c120e9b32c217290b80bad1a0729fbb2c/docs/stb_howto.txt for more info.
 */

#ifdef MEM_ARENA_GIMME_ALL_MAKE_STATIC
  #define MEM_ARENA_GIMME_ALL
  #define MEM_ARENA_MAKE_STATIC
#endif // MEM_ARENA_GIMME_ALL_MAKE_STATIC

#ifdef MEM_ARENA_GIMME_ALL
  #define MEM_ARENA_IMPLEMENTATION
  #define MEM_ARENA_GIMME_DYNAMIC_ARRAY
  #define MEM_ARENA_GIMME_STRING_BUILDER
#endif // MEM_ARENA_GIMME_ALL

#ifdef MEM_ARENA_GIMME_ALL_DEC
  #define MEM_ARENA_DYNAMIC_ARRAY
  #define MEM_ARENA_STRING_BUILDER
#endif // MEM_ARENA_GIMME_ALL_DEC

#ifdef MEM_ARENA_MAKE_STATIC
  #define MEM_ARENA_DEF static inline
  #define MEM_ARENA_DEC static inline

  #define MEM_ARENA_IMPLEMENTATION

  #ifdef MEM_ARENA_DYNAMIC_ARRAY
    #define MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION
  #endif // MEM_ARENA_DYNAMIC_ARRAY

  #ifdef MEM_ARENA_STRING_BUILDER
    #define MEM_ARENA_STRING_BUILDER_IMPLEMENTATION
  #endif // MEM_ARENA_STRING_BUILDER
#else
  #define MEM_ARENA_DEF
  #define MEM_ARENA_DEC extern
#endif // MEM_ARENA_MAKE_STATIC

#ifdef MEM_ARENA_GIMME_DYNAMIC_ARRAY
  #define MEM_ARENA_DYNAMIC_ARRAY
  #define MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION
#endif // MEM_ARENA_GIMME_DYNAMIC_ARRAY

#ifdef MEM_ARENA_GIMME_STRING_BUILDER
  #define MEM_ARENA_STRING_BUILDER
  #define MEM_ARENA_STRING_BUILDER_IMPLEMENTATION
#endif // MEM_ARENA_GIMME_STRING_BUILDER

#ifdef MEM_ARENA_STRING_BUILDER_IMPLEMENTATION
  #define MEM_ARENA_STRING_BUILDER
  #define MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION
#endif // MEM_ARENA_STRING_BUILDER_IMPLEMENTATION
#ifdef MEM_ARENA_STRING_BUILDER
  #define MEM_ARENA_DYNAMIC_ARRAY
#endif // MEM_ARENA_STRING_BUILDER
#ifdef MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION
  #define MEM_ARENA_DYNAMIC_ARRAY
#endif // MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION

#ifdef __linux__
  #define MEM_ARENA_IS_LINUX

  #include <sys/mman.h>
  #include <unistd.h>
#elif defined(_WIN32)
  #define MEM_ARENA_IS_WINDOWS

  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif // WIN32_LEAN_AND_MEAN

  #include <windows.h>
#else
  #error "Unsupported Operating System"
#endif // OPERATING SYSTEM

#ifndef MEM_ARENA_H_
#define MEM_ARENA_H_

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct mem_arena mem_arena;
struct mem_arena {
  uint32_t page_size;                      // system page size
  size_t reserve;                          // total virtual memory reserved
  size_t commit;                           // virtual memory to commit at a time
  size_t pos;                              // current pointer position
  size_t commit_pos;                       // current commit position
  void (*handle_oom)(mem_arena *, size_t); // function pointer to an oom error handler; ideally exits program
  const char *name;                        // optionally give your arena a name; must outlive arena.
};

typedef struct {
  uint8_t dummy;                            // dummy field for initialization
  size_t reserve_size;                      // default: 1 GiB
  size_t commit_size;                       // default: 2 MiB
  void (*oom_handler)(mem_arena *, size_t); // default: arena_oom_handler()
  const char *name;                         // default: NULL
} arena__init_params;

typedef struct {
  uint8_t dummy; // dummy field for initialization
  size_t align;  // default: MEM_ARENA_ALIGN
} arena__alloc_params;

#define MEM_ARENA_ALIGN_UP_POW2(n, p) (((size_t)(n) + ((size_t)(p) - 1)) & (~((size_t)(p) - 1)))
#define MEM_ARENA_ALIGN_UP(n, a)      ((((size_t)(n) + ((size_t)(a) - 1)) / (size_t)(a)) * (size_t)(a))
#define MEM_ARENA_BASE_POS            (sizeof(mem_arena))
#define MEM_ARENA_ALIGN               (alignof(max_align_t))

#define KiB(n) ((size_t)(n) << 10)
#define MiB(n) ((size_t)(n) << 20)
#define GiB(n) ((size_t)(n) << 30)

#define arena_init(...) arena__initialization_impl((arena__init_params){.dummy = 0, __VA_ARGS__})

#define arena_alloc(arena, size, ...)                                                                                  \
  arena__allocation_impl((arena), (arena__alloc_params){.dummy = 0, __VA_ARGS__}, (size), false)

#define arena_calloc(arena, count, size, ...)                                                                          \
  arena__callocation_impl((arena), (arena__alloc_params){.dummy = 0, __VA_ARGS__}, (count), (size))

#define arena_realloc(arena, ptr, old_size, new_size, ...)                                                             \
  arena__reallocation_impl((arena), (arena__alloc_params){.dummy = 0, __VA_ARGS__}, (ptr), (old_size), (new_size))

MEM_ARENA_DEC void arena_oom_handler(mem_arena *, size_t);
MEM_ARENA_DEC void arena_delete(mem_arena *);
MEM_ARENA_DEC void arena_rewind(mem_arena *, size_t);
MEM_ARENA_DEC void arena_clear(mem_arena *);
MEM_ARENA_DEC void arena_reset(mem_arena *);

MEM_ARENA_DEC mem_arena *arena__initialization_impl(arena__init_params);
MEM_ARENA_DEC void *arena__allocation_impl(mem_arena *, arena__alloc_params, size_t, bool);
MEM_ARENA_DEC void *arena__callocation_impl(mem_arena *, arena__alloc_params, size_t, size_t);
MEM_ARENA_DEC void *arena__reallocation_impl(mem_arena *, arena__alloc_params, void *, size_t, size_t);
MEM_ARENA_DEC void arena__clearing_impl(mem_arena *, bool);
MEM_ARENA_DEC uint32_t arena__sys_get_pagesize(void);
MEM_ARENA_DEC void *arena__sys_mem_reserve(size_t);
MEM_ARENA_DEC bool arena__sys_mem_commit(void *, size_t);
MEM_ARENA_DEC bool arena__sys_mem_decommit(void *, size_t);
MEM_ARENA_DEC bool arena__sys_mem_release(void *, size_t);

#endif // MEM_ARENA_H_

#if defined(MEM_ARENA_IMPLEMENTATION) && !defined(MEM_ARENA_IMPLEMENTATION_GUARD)
#define MEM_ARENA_IMPLEMENTATION_GUARD

MEM_ARENA_DEF void arena_oom_handler(mem_arena *arena, size_t requested_size) {
  fprintf(stderr,
          "[mem_arena%s%s] OUT OF MEMORY\n"
          " | Attempted to allocate: %zu bytes\n"
          " | Remaining capacity   : %zu bytes\n",
          (arena->name) ? ": " : "",
          (arena->name) ? arena->name : "",
          requested_size,
          (arena->reserve - arena->pos));

  exit(EXIT_FAILURE);
} /* arena_oom_handler() */

MEM_ARENA_DEF void arena_delete(mem_arena *arena) {
  if (arena) arena__sys_mem_release(arena, arena->reserve);
} /* arena_delete() */

MEM_ARENA_DEF void arena_rewind(mem_arena *arena, size_t pos) {
  if (!arena) return;
  size_t new_pos = (pos < MEM_ARENA_BASE_POS) ? MEM_ARENA_BASE_POS : pos;
  if (new_pos < arena->pos) {
    arena->pos = new_pos;
  }
} /* arena_rewind() */

MEM_ARENA_DEF void arena_clear(mem_arena *arena) {
  arena__clearing_impl(arena, false);
} /* arena_clear() */

MEM_ARENA_DEF void arena_reset(mem_arena *arena) {
  arena__clearing_impl(arena, true);
} /* arena_reset() */

MEM_ARENA_DEF mem_arena *arena__initialization_impl(arena__init_params params) {
  if (!params.reserve_size) params.reserve_size = GiB(1);
  if (!params.commit_size) params.commit_size = MiB(2);
  if (!params.oom_handler) params.oom_handler = arena_oom_handler;

  if (params.commit_size > params.reserve_size) params.commit_size = params.reserve_size;

  uint32_t page_size = arena__sys_get_pagesize();
  if ((!page_size) || ((page_size & (page_size - 1)) != 0)) return NULL;

  params.reserve_size += MEM_ARENA_BASE_POS;
  params.commit_size = (params.commit_size < page_size) ? page_size : params.commit_size;

  params.reserve_size = MEM_ARENA_ALIGN_UP_POW2(params.reserve_size, page_size);
  params.commit_size = MEM_ARENA_ALIGN_UP_POW2(params.commit_size, page_size);

  mem_arena *arena = arena__sys_mem_reserve(params.reserve_size);
  if (!arena) return NULL;
  if (!arena__sys_mem_commit(arena, page_size)) {
    arena__sys_mem_release(arena, params.reserve_size);
    return NULL;
  }

  arena->page_size = page_size;
  arena->reserve = params.reserve_size;
  arena->commit = params.commit_size;
  arena->pos = MEM_ARENA_BASE_POS;
  arena->commit_pos = page_size;
  arena->name = params.name;
  arena->handle_oom = params.oom_handler;
  return arena;
} /* arena__initialization_impl() */

MEM_ARENA_DEF void *arena__allocation_impl(mem_arena *arena, arena__alloc_params params, size_t size, bool zero_out) {
  if (!arena) return NULL;

  if (!params.align) params.align = MEM_ARENA_ALIGN;
  if ((params.align & (params.align - 1)) != 0) return NULL;

  size_t pos_align = MEM_ARENA_ALIGN_UP_POW2(arena->pos, params.align);
  size_t new_pos = pos_align + size;

  if ((new_pos < pos_align) || (new_pos > arena->reserve)) goto error_oom;

  if (new_pos > arena->commit_pos) {
    size_t new_commit_pos = MEM_ARENA_ALIGN_UP(new_pos, arena->commit);
    new_commit_pos = (new_commit_pos < arena->reserve) ? new_commit_pos : arena->reserve;

    uint8_t *mem = (uint8_t *)arena + arena->commit_pos;
    size_t commit_size = new_commit_pos - arena->commit_pos;

    if (!arena__sys_mem_commit(mem, commit_size)) goto error_oom;
    arena->commit_pos = new_commit_pos;
  }

  arena->pos = new_pos;

  uint8_t *out = (uint8_t *)arena + pos_align;
  if (zero_out) memset(out, 0, size);

  return out;

error_oom:
  arena->handle_oom(arena, size);
  return NULL;
} /* arena__allocation_impl() */

MEM_ARENA_DEF void *arena__callocation_impl(mem_arena *arena, arena__alloc_params params, size_t count, size_t size) {
  if ((!size) || (!arena)) return NULL;
  if (count > SIZE_MAX / size) goto error_oom;
  return arena__allocation_impl(arena, params, count * size, true);

error_oom:
  arena->handle_oom(arena, SIZE_MAX);
  return NULL;
} /* arena__callocation_impl */

MEM_ARENA_DEF void *arena__reallocation_impl(mem_arena *arena, arena__alloc_params params, void *ptr, size_t old_size,
                                             size_t new_size) {
  if (!arena) return NULL;
  if ((!ptr) || (!old_size)) return arena__allocation_impl(arena, params, new_size, false);
  if (new_size <= old_size) return ptr;
  size_t ptr_offset = (size_t)((uint8_t *)ptr - (uint8_t *)arena);
  bool is_most_recent = (ptr_offset + old_size == arena->pos);

  if (is_most_recent) {
    if (!arena__allocation_impl(arena, (arena__alloc_params){.align = 1}, new_size - old_size, false)) return NULL;
    return ptr;
  }

  void *new_ptr = arena__allocation_impl(arena, params, new_size, false);
  if (new_ptr) memcpy(new_ptr, ptr, old_size);
  return new_ptr;
} /* arena__reallocation_impl() */

MEM_ARENA_DEF void arena__clearing_impl(mem_arena *arena, bool do_decommit) {
  if (!arena) return;

  if (do_decommit) {
    if (arena->commit_pos > arena->page_size) {
      size_t decommit_size = arena->commit_pos - arena->page_size;
      uint8_t *decommit_ptr = (uint8_t *)arena + arena->page_size;

      arena__sys_mem_decommit(decommit_ptr, decommit_size);
      arena->commit_pos = arena->page_size;
    }
  }

  arena_rewind(arena, MEM_ARENA_BASE_POS);
} /* arena__clearing_impl() */

MEM_ARENA_DEF uint32_t arena__sys_get_pagesize(void) {
#ifdef MEM_ARENA_IS_LINUX
  return (uint32_t)sysconf(_SC_PAGESIZE);
#elif defined(MEM_ARENA_IS_WINDOWS)
  SYSTEM_INFO sys_info = {0};
  GetSystemInfo(&sys_info);
  return sys_info.dwPageSize;
#endif
} /* arena__sys_get_pagesize() */

MEM_ARENA_DEF void *arena__sys_mem_reserve(size_t size) {
#ifdef MEM_ARENA_IS_LINUX
  void *out = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
  return (out == MAP_FAILED) ? NULL : out;
#elif defined(MEM_ARENA_IS_WINDOWS)
  return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
#endif
} /* arena__sys_mem_reserve() */

MEM_ARENA_DEF bool arena__sys_mem_commit(void *ptr, size_t size) {
#ifdef MEM_ARENA_IS_LINUX
  return (mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0);
#elif defined(MEM_ARENA_IS_WINDOWS)
  return (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE));
#endif
} /* arena__sys_mem_commit() */

MEM_ARENA_DEF bool arena__sys_mem_decommit(void *ptr, size_t size) {
#ifdef MEM_ARENA_IS_LINUX
  if (mprotect(ptr, size, PROT_NONE) != 0) return false;
  return (madvise(ptr, size, MADV_DONTNEED) == 0);
#elif defined(MEM_ARENA_IS_WINDOWS)
  return VirtualFree(ptr, size, MEM_DECOMMIT);
#endif
} /* arena__sys_mem_decommit() */

MEM_ARENA_DEF bool arena__sys_mem_release(void *ptr, size_t size) {
#ifdef MEM_ARENA_IS_LINUX
  return (munmap(ptr, size) == 0);
#elif defined(MEM_ARENA_IS_WINDOWS)
  (void)size;
  return VirtualFree(ptr, 0, MEM_RELEASE);
#endif
} /* arena__sys_mem_release() */

#endif // MEM_ARENA_IMPLEMENTATION

#if defined(MEM_ARENA_STRING_BUILDER) && !defined(MEM_ARENA_STRING_BUILDER_GUARD)
#define MEM_ARENA_STRING_BUILDER_GUARD

#include <ctype.h>

typedef char *strb; // string builder

typedef struct {
  uint8_t dummy;   // dummy field for initialization
  bool ignore_ord; // default: false
} sb__remove_params;

#define build_strb(arena, str)       sb__build_strb_impl((arena), (str), strlen(str))
#define sb_null_terminate(arena, sb) sb__push_null_impl((arena), &(sb))
#define sb_tolower(sb)               sb__letter_case_shift_impl(&(sb), true)
#define sb_toupper(sb)               sb__letter_case_shift_impl(&(sb), false)
#define sb_insert(arena, sb, c, n)   sb__insertion_impl((arena), &(sb), (c), (n))
#define sb_concat(arena, sb1, sb2)   sb__concatenation_impl((arena), &(sb1), (sb2))
#define sb_append(arena, sb, c)      sb__insertion_impl((arena), &(sb), (c), sb_strlen(sb))
#define sb_remove(sb, n, ...)        sb__removal_impl(&(sb), (n), (sb__remove_params){.dummy = 0, __VA_ARGS__})
#define sb_reverse(sb)               sb__reversal_impl(&(sb))
#define sb_ltrim(sb)                 sb__trim_impl(&(sb), true)
#define sb_rtrim(sb)                 sb__trim_impl(&(sb), false)
#define sb_trim(sb)                  sb__trim_both_impl(&(sb))

MEM_ARENA_DEC size_t sb_strlen(strb);
MEM_ARENA_DEC bool sb_isempty(strb);
MEM_ARENA_DEC bool sb_isblank(strb);
MEM_ARENA_DEC bool sb_equals(strb, strb);
MEM_ARENA_DEC bool sb_equals_ic(strb, strb);
MEM_ARENA_DEC int sb_compare(strb, strb);
MEM_ARENA_DEC uint32_t sb_hash(strb);

MEM_ARENA_DEC strb sb__build_strb_impl(mem_arena *, const char *, size_t len);
MEM_ARENA_DEC void sb__push_null_impl(mem_arena *, strb *);
MEM_ARENA_DEC void sb__pop_null_impl(strb *);
MEM_ARENA_DEC void sb__letter_case_shift_impl(strb *, bool);

MEM_ARENA_DEC bool sb__insertion_impl(mem_arena *, strb *, char, size_t);
MEM_ARENA_DEC bool sb__concatenation_impl(mem_arena *, strb *, strb);
MEM_ARENA_DEC bool sb__removal_impl(strb *, size_t, sb__remove_params);
MEM_ARENA_DEC bool sb__reversal_impl(strb *);
MEM_ARENA_DEC bool sb__trim_impl(strb *, bool);
MEM_ARENA_DEC bool sb__trim_both_impl(strb *);

#endif // MEM_ARENA_STRING_BUILDER

#if defined(MEM_ARENA_DYNAMIC_ARRAY) && !defined(MEM_ARENA_DYNAMIC_ARRAY_GUARD)
#define MEM_ARENA_DYNAMIC_ARRAY_GUARD

typedef union {
  struct {
    size_t count;
    size_t cap;
  } data;
  max_align_t align;
} darr_header;

#define DYNAMIC_ARRAY_INIT_CAPACITY (8)

#define darr_init(arena, type, size) darr__initialization_impl((arena), sizeof(type), (size))

#define darr_len(arr)  ((arr) ? ((darr_header *)(arr) - 1)->data.count : 0)
#define darr_cap(arr)  ((arr) ? ((darr_header *)(arr) - 1)->data.cap : 0)
#define darr_last(arr) ((arr)[darr_len(arr) - 1])
#define darr_pop(arr)  (void)(((arr) && (darr_len(arr) > 0)) ? --((darr_header *)(arr) - 1)->data.count : 0)

#define darr_rev(arr)                                                                                                  \
  ({                                                                                                                   \
  bool darr_rev__success = false;                                                                                      \
  if (arr) {                                                                                                           \
    size_t darr_rev__len = darr_len(arr);                                                                              \
    if (darr_rev__len > 1) {                                                                                           \
      darr_rev__success = true;                                                                                        \
      typeof(arr) darr_rev__left = (arr);                                                                              \
      typeof(arr) darr_rev__right = darr_rev__left + (darr_rev__len - 1);                                              \
      for (; darr_rev__left < darr_rev__right; darr_rev__left++, darr_rev__right--) {                                  \
        typeof(*(arr)) darr_rev__tmp = (*darr_rev__left);                                                              \
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
    void *darr_push__tmp = darr__growth_impl((arena), (arr), sizeof(*(arr)));                                          \
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
    void *darr_reserve__tmp = darr__reserve_impl((arena), (arr), sizeof(*(arr)), (size));                              \
    if (darr_reserve__tmp) {                                                                                           \
      (arr) = darr_reserve__tmp;                                                                                       \
      darr_reserve__success = true;                                                                                    \
    }                                                                                                                  \
  }                                                                                                                    \
  darr_reserve__success;                                                                                               \
  })

MEM_ARENA_DEC void *darr__initialization_impl(mem_arena *, size_t, size_t);
MEM_ARENA_DEC void *darr__growth_impl(mem_arena *, void *, size_t);
MEM_ARENA_DEC void *darr__reserve_impl(mem_arena *, void *, size_t, size_t);

#endif // MEM_ARENA_DYNAMIC_ARRAY

#if defined(MEM_ARENA_STRING_BUILDER_IMPLEMENTATION) && !defined(MEM_ARENA_STRING_BUILDER_IMPLEMENTATION_GUARD)
#define MEM_ARENA_STRING_BUILDER_IMPLEMENTATION_GUARD

MEM_ARENA_DEF size_t sb_strlen(strb sb) {
  if (!sb) return 0;
  size_t len = darr_len(sb);
  if (!len) return 0;
  if (darr_last(sb) == '\0') len--;

  return len;
} /* sb_strlen() */

MEM_ARENA_DEF bool sb_isempty(strb sb) {
  return ((!sb) || (sb_strlen(sb) == 0));
} /* sb_isempty() */

MEM_ARENA_DEF bool sb_isblank(strb sb) {
  if (sb_isempty(sb)) return true;

  for (size_t n = 0; n < sb_strlen(sb); n++) {
    if (!isspace((unsigned char)sb[n])) return false;
  }

  return true;
} /* sb_isblank() */

MEM_ARENA_DEF bool sb_equals(strb a, strb b) {
  size_t a_len = sb_strlen(a);
  size_t b_len = sb_strlen(b);

  if (a_len != b_len) return false;
  else if (sb_isempty(a)) return true;

  return (memcmp(a, b, a_len) == 0);
} /* sb_equals() */

MEM_ARENA_DEF bool sb_equals_ic(strb a, strb b) {
  size_t a_len = sb_strlen(a);
  size_t b_len = sb_strlen(b);

  if (a_len != b_len) return false;
  else if (sb_isempty(a)) return true;

  for (size_t n = 0; n < a_len; n++) {
    if (toupper((unsigned char)a[n]) != toupper((unsigned char)b[n])) return false;
  }

  return true;
} /* sb_equals_ic() */

MEM_ARENA_DEF int sb_compare(strb a, strb b) {
  size_t a_len = sb_strlen(a);
  size_t b_len = sb_strlen(b);

  size_t cmp_len = (a_len < b_len) ? a_len : b_len;
  int cmp = (cmp_len) ? memcmp(a, b, cmp_len) : 0;

  if (cmp) return cmp;
  if (a_len != b_len) return (a_len < b_len) ? -1 : 1;
  return 0;
} /* sb_compare() */

MEM_ARENA_DEF uint32_t sb_hash(strb sb) { /* MurmurHash3 */
  if (sb_isempty(sb)) return 0;

  uint32_t hash = 0;
  uint32_t k;
  size_t len = sb_strlen(sb);

  for (size_t n = len >> 2; n; n--) {
    memcpy(&k, sb, sizeof(uint32_t));
    sb += sizeof(uint32_t);

    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;

    hash ^= k;
    hash = (hash << 13) | (hash >> 19);
    hash = hash * 5 + 0xe6546b64;
  }

  k = 0;
  for (size_t n = len & 3; n; n--) {
    k <<= 8;
    k |= (unsigned char)sb[n - 1];
  }

  k *= 0xcc9e2d51;
  k = (k << 15) | (k >> 17);
  k *= 0x1b873593;

  hash ^= k;
  hash ^= len;
  hash ^= hash >> 16;
  hash *= 0x85ebca6b;
  hash ^= hash >> 13;
  hash *= 0xc2b2ae35;
  hash ^= hash >> 16;
  return hash;
} /* sb_hash() */

MEM_ARENA_DEF strb sb__build_strb_impl(mem_arena *arena, const char *str, size_t len) {
  if ((!arena) || (!str)) return NULL;
  strb sb = darr__initialization_impl(arena, sizeof(char), len + 1);
  if (!sb) return NULL;

  memcpy(sb, str, len + 1);
  ((darr_header *)sb - 1)->data.count = len + 1;
  return sb;
} /* sb__build_strb_impl() */

MEM_ARENA_DEF void sb__push_null_impl(mem_arena *arena, strb *sb) {
  if (!sb) return;
  if (!(*sb)) return;

  size_t len = darr_len(*sb);
  if (!len) return;
  if (darr_last(*sb) != '\0') darr_push(arena, (*sb), '\0');
} /* sb__push_null_impl() */

MEM_ARENA_DEF void sb__pop_null_impl(strb *sb) {
  if (!sb) return;
  if (!(*sb)) return;

  size_t len = darr_len(*sb);
  if (!len) return;
  if (darr_last(*sb) == '\0') darr_pop(*sb);
} /* sb__pop_null_impl() */

MEM_ARENA_DEF void sb__letter_case_shift_impl(strb *sb, bool to_lower) {
  if (!sb) return;
  if (sb_isempty(*sb)) return;

  for (size_t n = 0; n < sb_strlen(*sb); n++) {
    if (to_lower) (*sb)[n] = tolower((unsigned char)(*sb)[n]);
    else (*sb)[n] = toupper((unsigned char)(*sb)[n]);
  }
} /* sb__letter_case_shift_impl() */

MEM_ARENA_DEF bool sb__insertion_impl(mem_arena *arena, strb *sb, char c, size_t n) {
  if (!sb) return false;
  if (!(*sb)) return false;
  sb__pop_null_impl(sb);

  size_t len = sb_strlen(*sb);
  size_t cap = darr_cap(*sb);
  if (n > len) n = len;

  if (len >= ((!cap) ? 0 : cap - 1)) {
    char *tmp = darr__growth_impl(arena, (*sb), sizeof(char));
    if (!tmp) {
      sb__push_null_impl(arena, sb);
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
  sb__push_null_impl(arena, sb);

  return true;
} /* sb__insertion_impl() */

MEM_ARENA_DEF bool sb__concatenation_impl(mem_arena *arena, strb *a, strb b) {
  if ((!a) || (!b)) return false;
  if (!(*a)) return false;
  if (sb_isempty(b)) return true;

  size_t a_len = sb_strlen(*a);
  size_t b_len = sb_strlen(b);

  if (SIZE_MAX - a_len < b_len) return false;

  size_t a_cap = darr_cap(*a);
  sb__pop_null_impl(a);
  size_t requested_size = a_len + b_len + 1;

  if (requested_size > a_cap) {
    size_t new_cap = requested_size > a_cap * 2 ? requested_size : a_cap * 2;
    char *tmp = darr__reserve_impl(arena, (*a), sizeof(char), new_cap);
    if (!tmp) {
      sb__push_null_impl(arena, a);
      return false;
    }
    (*a) = tmp;
  }

  memmove((*a) + a_len, b, b_len);
  ((darr_header *)(*a) - 1)->data.count += b_len;
  sb__push_null_impl(arena, a);

  return true;
} /* sb__concatenation_impl() */

MEM_ARENA_DEF bool sb__removal_impl(strb *sb, size_t n, sb__remove_params params) {
  if (!sb) return false;
  if (sb_isempty(*sb)) return false;
  size_t len = sb_strlen(*sb);

  if (n >= len) return false;

  if (len == 1) {
    (*sb)[0] = '\0';
    ((darr_header *)(*sb) - 1)->data.count = 1;
    return true;
  }

  sb__pop_null_impl(sb);

  if (params.ignore_ord) {
    if (!darr_rem((*sb), n)) return false;
  } else {
    if (!darr_rem_ord((*sb), n)) return false;
  }

  (*sb)[len - 1] = '\0';
  ((darr_header *)(*sb) - 1)->data.count++;
  return true;
} /* sb__removal_impl() */

MEM_ARENA_DEF bool sb__reversal_impl(strb *sb) {
  if (!sb) return false;
  if (sb_isempty(*sb)) return false;

  size_t len = sb_strlen(*sb);
  if (len <= 1) return true;

  char *left = (*sb);
  char *right = left + (len - 1);

  for (; left < right; left++, right--) {
    char tmp = (*left);
    (*left) = (*right);
    (*right) = tmp;
  }

  return true;
} /* sb__reversal_impl() */

MEM_ARENA_DEF bool sb__trim_impl(strb *sb, bool trim_left) {
  if (!sb) return false;
  if (sb_isempty(*sb)) return true;

  size_t len = sb_strlen(*sb);
  if (len == 1) {
    if (isspace((unsigned char)(*sb)[0])) {
      darr_pop(*sb);
      ((darr_header *)(*sb) - 1)->data.count = 1;
      (*sb)[0] = '\0';
      return true;
    }
  } else if (sb_isblank(*sb)) {
    ((darr_header *)(*sb) - 1)->data.count = 1;
    (*sb)[0] = '\0';
    return true;
  }

  if (trim_left) {
    size_t whitespace_count = 0;
    sb__pop_null_impl(sb);
    for (size_t n = 0; n < len; n++) {
      if (!isspace((unsigned char)(*sb)[n])) break;
      whitespace_count++;
    }

    memmove((*sb), (*sb) + whitespace_count, len - whitespace_count);
    (*sb)[len - whitespace_count] = '\0';
    ((darr_header *)(*sb) - 1)->data.count = len - whitespace_count + 1;
  } else {
    sb__pop_null_impl(sb);
    while ((len > 0) && (isspace((unsigned char)(*sb)[--len]))) darr_pop(*sb);
    (*sb)[++len] = '\0';
    ((darr_header *)(*sb) - 1)->data.count++;
  }

  return true;
} /* sb__trim_impl() */

MEM_ARENA_DEF bool sb__trim_both_impl(strb *sb) {
  return (sb__trim_impl(sb, true) && sb__trim_impl(sb, false));
} /* sb__trim_both_impl() */

#endif // MEM_ARENA_STRING_BUILDER_IMPLEMENTATION

#if defined(MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION) && !defined(MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION_GUARD)
#define MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION_GUARD

MEM_ARENA_DEF void *darr__initialization_impl(mem_arena *arena, size_t elem_size, size_t count) {
  if (!elem_size) return NULL;
  if ((elem_size != 0) && (count > SIZE_MAX / elem_size)) return NULL;
  size_t total_size = sizeof(darr_header) + (elem_size * count);
  darr_header *header = arena_alloc(arena, total_size);
  if (!header) return NULL;

  header->data.count = 0;
  header->data.cap = count;

  return header + 1;
} /* darr__initialization_impl() */

MEM_ARENA_DEF void *darr__growth_impl(mem_arena *arena, void *arr, size_t elem_size) {
  if (!elem_size) return NULL;
  darr_header *header;

  if (!arr) {
    if ((elem_size != 0) && (DYNAMIC_ARRAY_INIT_CAPACITY > SIZE_MAX / elem_size)) return NULL;
    header = arena_alloc(arena, sizeof(darr_header) + (elem_size * DYNAMIC_ARRAY_INIT_CAPACITY));
    if (!header) return NULL;

    header->data.count = 0;
    header->data.cap = DYNAMIC_ARRAY_INIT_CAPACITY;
  } else {
    header = (darr_header *)arr - 1;
    if (header->data.cap > SIZE_MAX / 2) return NULL;

    size_t new_cap = (header->data.cap) ? header->data.cap * 2 : DYNAMIC_ARRAY_INIT_CAPACITY;
    if ((elem_size != 0) && (new_cap > SIZE_MAX / elem_size)) return NULL;

    size_t old_size = sizeof(darr_header) + (elem_size * header->data.cap);
    size_t new_size = sizeof(darr_header) + (elem_size * new_cap);

    darr_header *new_header = arena_realloc(arena, header, old_size, new_size);
    if (!new_header) return NULL;

    header = new_header;
    header->data.cap = new_cap;
  }
  return header + 1;
} /* darr__growth_impl() */

MEM_ARENA_DEF void *darr__reserve_impl(mem_arena *arena, void *arr, size_t elem_size, size_t size) {
  if (!size) return NULL;
  if (!arr) return NULL;

  darr_header *header = (darr_header *)arr - 1;
  if (header->data.cap >= size) return arr;

  if ((elem_size != 0) && (size > SIZE_MAX / elem_size)) return NULL;

  size_t old_size = sizeof(darr_header) + (elem_size * header->data.cap);
  size_t new_size = sizeof(darr_header) + (elem_size * size);

  darr_header *new_header = arena_realloc(arena, header, old_size, new_size);
  if (!new_header) return NULL;

  header = new_header;
  header->data.cap = size;

  return header + 1;
} /* darr__reserve_impl() */

#endif // MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION

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
