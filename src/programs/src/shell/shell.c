#include "shell.h"

void main() {
    shell_init();
    shell_print_help();
    shell_loop();
}

void shell_init() {
    // empty
}

void shell_loop() {
    while(1) {
        shell_print_prompt();
        char command[256];
        // Read command from input (this is a placeholder, actual implementation needed)
        // read_command(command, sizeof(command));
        
        // Execute the command
        shell_exec(command);
    }
}

void shell_exec(char* command) {
    // empty
}

void shell_print_prompt() {
    // empty
}

void shell_print_help() {
    // empty
}
