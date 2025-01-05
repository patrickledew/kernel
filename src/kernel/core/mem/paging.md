# Paging

In order to enable paging:
- Set CR0.PG = 1
- CR3 must contain the address of the first paging structure

Paging structures
- 1024 4-byte entries = 4kb
- Each entry points to another paging structure or the address of a page frame

> The translation process uses 10 bits at a time from a 32-bit linear address. Bits 31:22 identify the first paging-structure entry and bits 21:12 identify a second. The latter identifies the page frame. Bits 11:0 of the linear address are the page offset within the 4-KByte page frame.


# Switching to kernel space from user space
- When doing this, there are two approaches


1. Turn off paging while executing kernel code. This would cause issues if the user program passed a virtual address, as we would manually have to look up that in the page tables.
2. Keep paging enabled, but kernel code will itself be mapped using the page table. Say, for example, we map the kernel (which sits at 0x10000 to 0x1FFFF in physical memory) to address 0xC0000000 in virtual memory. When we jump back to the kernel, we would be using addresses 0xC0000000-0xC000FFFF.
    - One problem with this is the physical memory manager, which relies on physical addressing. For calls to alloc() or free(), we would have to temporarily disable paging in order to use that.




