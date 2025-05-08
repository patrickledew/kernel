#include "panic.h"
#include "logging.h"

__attribute__((noreturn))
void panic(char* message) {
    log_error("FATAL: Kernel panic. Halting.");
    log_error(message);

    __asm__("cli; hlt;");
    while (1) {
    }
}