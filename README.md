# `mem_arena.h` — Header-only library for vitual memory arenas

## Usage

Macros that enable function definitions (excluding the MAKE_STATIC macros) must only be defined once in one source file. See [this](https://github.com/nothings/stb/blob/f58f558c120e9b32c217290b80bad1a0729fbb2c/docs/stb_howto.txt) for more info.

Define `_DEFAULT_SOURCE` beforehand.

### Flags

- `MEM_ARENA_IMPLEMENTATION`: enable function definitions
- `MEM_ARENA_USE_DEFAULT_OOM_HANDLER`: set arena_oom_handler() as default oom handler function
- `MEM_ARENA_DYNAMIC_ARRAY`: enable dynamic array function declarations
- `MEM_ARENA_DYNAMIC_ARRAY_IMPLEMENTATION`: enable dynamic array function definitions
- `MEM_ARENA_GIMME_DYNAMIC_ARRAY`: enable dynamic array function declarations + definitions
- `MEM_ARENA_STRING_BUILDER`: enable string function declarations
- `MEM_ARENA_STRING_BUILDER_IMPLEMENTATION`: enable string function definitions
- `MEM_ARENA_GIMME_STRING`: enable string function declarations + definitions
- `MEM_ARENA_GIMME_ALL`: define all of the above
- `MEM_ARENA_GIMME_ALL_DEC`: include all declarations; no definitions
- `MEM_ARENA_MAKE_STATIC`: make all functions static inline + include definitions
- `MEM_ARENA_GIMME_ALL_MAKE_STATIC`: same as `MEM_ARENA_GIMME_ALL` but all functions are static inline

## License

This software is available under 2 licenses. See [LICENSE](LICENSE) for more information.
