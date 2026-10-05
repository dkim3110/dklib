# `mem_arena.h` — Virtual Memory Arena Allocator

Header-only library à la [stb](https://github.com/nothings/stb) for a virtual memory [arena allocator](https://en.wikipedia.org/wiki/Region-based_memory_management) written in C.

## Usage

Macros that enable function definitions (excluding the MAKE_STATIC macros) must only be defined once in one source file before including the header. See [here](https://github.com/nothings/stb/blob/f58f558c120e9b32c217290b80bad1a0729fbb2c/docs/stb_howto.txt) for more information.

This library requires GCC or Clang with GNU C extensions, so compile with `-std=gnu11`. It targets Linux and Windows only. Windows support is written against the Win32 API but untested.

This library does not concern itself with C++ support.

### Flags

- `MEM_ARENA_IMPLEMENTATION`: enable function definitions
- `MEM_ARENA_DYNAMIC_ARRAY`: enable dynamic array function declarations
- `MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION`: enable dynamic array function definitions
- `MEM_ARENA_GIMME_DYNAMIC_ARRAY`: enable dynamic array function declarations + definitions
- `MEM_ARENA_STRING_BUILDER`: enable string function declarations
- `MEM_ARENA_STRING_BUILDER_IMPLEMENTATION`: enable string function definitions
- `MEM_ARENA_GIMME_STRING_BUILDER`: enable string function declarations + definitions
- `MEM_ARENA_GIMME_ALL`: include all function definitions
- `MEM_ARENA_GIMME_ALL_DEC`: include all declarations; no definitions
- `MEM_ARENA_IMPLEMENTATION_MAKE_STATIC`: same as `MEM_ARENA_IMPLEMENTATION` but all functions are static inline
- `MEM_ARENA_GIMME_ALL_MAKE_STATIC`: same as `MEM_ARENA_GIMME_ALL` but all functions are static inline

### Optional Function Parameters

Several function-like macros (`arena_init()`, `arena_alloc()`, `sb_remove()`, etc.) accept optional parameters. You can set custom values:

```C
mem_arena *arena = arena_init(.reserve_size = MiB(500), .name = "Arena");
```

Consult the parameter structs in the header for a full list of defaults.

### OOM Handler Function

This library provides a default oom handler that exits the program with an error code. You can use your own handler if you so choose.

```C
mem_arena *arena = arena_init(.oom_handler = custom_function);
```

### Thread Safety

Not safe at all.

### Example Code

```C
#define MEM_ARENA_GIMME_ALL
#include "mem_arena.h"

#include <stdio.h>

int main(void) {
  mem_arena *arena = arena_init(.name = "Main Arena"); /* Initialize arena */
  if (!arena) return 1;
  
  strb text = build_strb(arena, "Hello, World"); /* Initialize string builder */
  size_t pos = arena_get_pos(arena);
  size_t *numbers = darr_init(arena, size_t, 10); /* Initialize dynamic array */
  
  for (size_t n = 0; n < 20; n++) {
    darr_push(arena, numbers, n); /* return values rarely need checking thanks to the oom handler */
  }
  
  sb_append(arena, text, '!');
  printf("%s\n", text);
  
  arena_rewind(arena, pos); /* rewind arena to saved position */
  arena_delete(arena); /* deallocate all at once */
  return 0;
}
```

## License

This software is available under 2 licenses. See [LICENSE](LICENSE) for more information.
