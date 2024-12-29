# How processes will work in this kernel

Several areas we'd need to work on
1. How do we represent processes in memory? How is a process structured?
2. How do we load a process's executable code/data into memory?
3. How do we interact with a process? - through syscalls. Need to define.
4. How is a processes memory mapped? Should we try experimenting without vmem first?


# 1. Memory of a process
- Processes need stack, heap (need to implement), data, and text regions.


+----------------------+
|     STACK            |
|                      |
+----------------------+
|   v    v     v   v   |
|                      |
|   ^    ^     ^   ^   |
+----------------------+
|     HEAP             |
|                      |
+----------------------+
| .data                |
|                      |
|                      |
|                      |
|                      |
+----------------------+
| .text                |
|                      |
+----------------------+

- The .data and .text sections in an ELF file would have to be moved to the correct positions in memory, if not in correct order already
- Other sections, such as .bss, may also need to be relocated
- The stack would have to be set up, and the heap would need to be managed by calls to a malloc implementation.

## How does the kernel know where to put it?
- We still need to find space to allocate for the process. We'd do this by reserving an area using alloc() (physical memory allocator) and using the returned address as the input for vmem mappings, reading in executable code, etc.
- Then we'd read the ELF headers to figure out where to put each section

https://en.wikipedia.org/wiki/Executable_and_Linkable_Format for details

## How will the kernel represent the process internally?
- We need a PCB (process control block) to keep track of information about the process.

This will include:
- PID
- Pointer to parent process
- Process state (waiting, ready, exiting, etc.)
- Program counter
- Registers
- Page table, memory limits, etc.
- I/O status (e.g. file descriptors being used)
- Anything else we need
