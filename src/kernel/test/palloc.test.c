#include "palloc.test.h"
#include "util/logging.h"
#include "core/mem/vmem.h"

void palloc_test() {
    palloc_init();

    uint32_t* pd = vmem_pd_create();
    // Assert palloc_stat returns full page count
    int pagecount = PALLOC_LIMIT / PAGE_SIZE;
    assert_u32(pagecount, palloc_stat());
    log_info("palloc_test: palloc_stat() passed.");

    // Assert palloc_find_next_free returns first page
    uint8_t* first_page = palloc_find_next_free();
    assert_u32(PALLOC_PHYS_BASE, (uint32_t)first_page);
    log_info("palloc_test: palloc_find_next_free() passed.");

    // Assert palloc respects VMA bounds
    uint8_t* vma = (uint8_t*)0xc0000000;
    assert_u32(-1, palloc(vma, PAGE_SIZE, pd));
    vma = (uint8_t*)0xfffff000;
    assert_u32(-1, palloc(vma, PAGE_SIZE, pd));
    // Also assert that the above calls didn't allocate anything
    assert_u32(pagecount, palloc_stat());

    log_info("palloc_test: palloc VMA bounds passed.");

    // Assert palloc allocates a page
    assert_u32(0, palloc((uint8_t*)0x00000000, PAGE_SIZE, pd));
    assert_u32(pagecount - 1, palloc_stat());
    log_number("PT entry", vmem_entry_get(pd, (uint8_t*)0x00000000), 16);

    uint32_t flags = vmem_entry_get(pd, (uint8_t*)0x00000000) & 0xFFF;
    assert_u32(PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE | PAGE_ENTRY_MASK_PALLOC_END, flags);
    
    log_info("palloc_test: palloc single page alloc passed.");

    // Assert palloc allocates multiple pages
    assert_u32(0, palloc((uint8_t*)0x00001000, PAGE_SIZE * 2, pd));
    assert_u32(pagecount - 3, palloc_stat());
    uint32_t flags_1 = vmem_entry_get(pd, (uint8_t*)0x00001000) & 0xFFF;
    uint32_t flags_2 = vmem_entry_get(pd, (uint8_t*)0x00002000) & 0xFFF;
    assert_u32(PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE, flags_1);
    assert_u32(PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE | PAGE_ENTRY_MASK_PALLOC_END, flags_2);

    log_info("palloc_test: palloc multi-page alloc passed.");

    // Assert pfree frees one page
    assert_u32(0, pfree((uint8_t*)0x00000000, pd));
    assert_u32(pagecount - 2, palloc_stat());
    assert_u32(0, vmem_entry_get(pd, (uint8_t*)0x00000000));

    log_info("palloc_test: pfree single page free passed.");

    // Assert pfree frees multiple pages
    assert_u32(0, pfree((uint8_t*)0x00001000, pd));
    assert_u32(pagecount, palloc_stat());
    assert_u32(0, vmem_entry_get(pd, (uint8_t*)0x00001000));
    assert_u32(0, vmem_entry_get(pd, (uint8_t*)0x00002000));

    log_info("palloc_test: pfree multi-page free passed.");

    // Assert pfree can free a block immediately before another block
    assert_u32(0, palloc((uint8_t*)0x00000000, PAGE_SIZE * 2, pd));
    assert_u32(0, palloc((uint8_t*)0x00002000, PAGE_SIZE, pd));
    assert_u32(0, pfree((uint8_t*)0x00000000, pd));
    assert_u32(0, vmem_entry_get(pd, (uint8_t*)0x00000000));
    assert_u32(0, vmem_entry_get(pd, (uint8_t*)0x00001000));
    assert_not_u32(0, vmem_entry_get(pd, (uint8_t*)0x00002000));

    log_info("palloc_test: pfree block before another block passed.");
}