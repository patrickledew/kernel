#include "malloc.test.h"
#include "util/logging.h"

void malloc_test() {
    // Setup up "PCB" which just uses kernel address space
    processes[0].page_directory = current_page_directory;
    processes[0].malloc_info = 0;
    log_info("malloc_test: malloc test starting.");
    malloc_impl(0x500, 0);
    malloc_impl(0x500, 0);
    log_info("malloc_test: malloc test ended.");

}