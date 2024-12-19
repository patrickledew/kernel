# Style Guidelines
I'm bad at writing good C code, and this kernel has mostly just been a way for me to learn things without caring too much about it being pretty. But, as this project has grown to more than just a couple files, it would be a not bad thing to establish some code style guidelines.

## Functions
Functions should be named with the following naming convention:

```void module_object_action(int* object);```

Some good examples might be `fs_fat_read()` or `disk_sector_write()`.

If there is no "object" associated with the action, we can drop it. E.g. `kernel_init()`.

## Types/Structs

With the exception of primitives, e.g. uint8_t or bool, types will be named in camel case e.g. `MyType`.

## Variables
Variables are named in `snake_case`.

## Other stuff
- ALWAYS use uints instead of ints unless a signed value is absolutely needed.
- Pointers to blocks of memory containing arbitrary data should be `uint8_t*`s.
- For actual ascii characters, use `char`,  but all other single-byte values should be `uint8_t`.