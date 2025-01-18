Processes - TODO
- [x] Build ELF parser, read in a program, and print out information
- [-] Using program header table, allocate pages and map them to virtual addresses given (need physical page allocator!)
    - Still need to allocate pages from PA 0x200000 onward
- [ ] Copy sections to these newly mapped pages
- [ ] Jump to entry point, and hope a DBZ error happens (this means it worked)


Process page allocator
- Should be able to use the same page alloc code we already wrote (bitmap mostly)
- Instead of using memory addresses directly, need to mark pages as available in page table first


1. parse elf
2. construct info about process (initial registers, etc.)
3. jump to usermode (we dont have that yet - but at least we should use separate page tables)
