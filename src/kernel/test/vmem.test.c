
#include "vmem.test.h"
#include "core/interrupts/interrupts.h"
#include "core/mem/vmem.h"
#include "core/mem/alloc.h"
#include "core/mem/memory.h"
#include "util/assert.h"
#include "util/logging.h"

bool page_fault_occurred = FALSE;
uint8_t* err_page;
uint32_t* pd;

void vmem_test_create_err_page() {
    err_page = (uint8_t*)alloc(PAGE_SIZE);
    memfill(err_page, PAGE_SIZE, 0xFF);
}

__attribute__((interrupt))
void vmem_test_isr_pagefault(InterruptFrame* frame, uint32_t error_code) {
    page_fault_occurred = TRUE;
    uint8_t* target_addr;
    __asm__("movl %%cr2, %%eax" : "=a"(target_addr)); // Get attempted address
    
    vmem_map(pd, KADDR_TO_PADDR(err_page), (uint8_t*)target_addr, 1, PAGE_ENTRY_MASK_READWRITE);
    vmem_load(pd);
}

#define assert_no_page_fault() assert_b(FALSE, page_fault_occurred);


#define assert_page_fault() assert_b(TRUE, page_fault_occurred); page_fault_occurred = FALSE;

void vmem_test() {
    log_info("vmem_test: running virtual memory tests.");
    vmem_test_create_err_page();
    // Register PF interrupt
    REG_ISR(0x0E, vmem_test_isr_pagefault);

    // Create page directory
    pd = vmem_pd_create();

    // 1) Map two virtual addresses to the same physical address.
    uint8_t* phys = (uint8_t*)0x10000;
    uint32_t* virt_a = (uint32_t*)0x12345000;
    uint32_t* virt_b = (uint32_t*)0xfefef000;

    assert_i(vmem_map(pd, phys, (uint8_t*)virt_a, 2, PAGE_ENTRY_MASK_READWRITE), 0);
    assert_i(vmem_map(pd, phys, (uint8_t*)virt_b, 3, PAGE_ENTRY_MASK_READWRITE), 0);

    vmem_load(pd);

    // fill a page with 1,2,3,4... and check that the other address has same data
    for (uint32_t i = 0; i < 1024; i++) {
        virt_a[i] = i;
        assert_u32(virt_b[i], i);
    }

    // 2) With above address, assert that accessing address causes page fault for A but not B
    int off = (2 * PAGE_SIZE + 50) / 4; // 2 pg + 50 B in, outside of mapped range for A

    // A
    uint32_t i = virt_a[off];
    assert_page_fault();
    // B
    i = virt_b[off];
    assert_no_page_fault();

    // 3) Unmap A, assert that accessing A causes page fault
    vmem_unmap(pd, (uint8_t*)virt_a, 2);
    i = virt_a[0];
    assert_page_fault();

    vmem_pd_destroy(pd);

    uint8_t color = print_color_get();
    print_color_set(0x0A);
    log_info("vmem_test: virtual memory tests completed successfully.");
    print_color_set(color);
}