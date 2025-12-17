#include "shell.h"
#include "strutil.h"
#include "builtins.h"
#define MAX_COMMAND_LENGTH 256
char current_directory[50] = "/";

void main() {
    shell_init();
    shell_print_help();
    shell_loop();
}

void shell_init() {
    // empty
}

void shell_loop() {
    char command[MAX_COMMAND_LENGTH] = {0};
    while(1) {
        shell_print_prompt();
        
        // Read one byte at a time from stdin and store in the command buffer.
        int cmd_len = 0;
        while (1) {
            int success = read(0, 1, command + cmd_len); // Read from stdin
            if (success < 0) {
                print("Error reading command.\n");
                return; // Exit on read error
            } else if (success == 0) {
                continue; // No input, continue reading
            }
            // Check each byte - check for backspace, newline, or carriage return
            if (command[cmd_len] == '\b') {
                if (cmd_len > 0) {
                    cmd_len--; // Remove last character
                    print("\b"); // Echo backspace
                }
                continue;
            } else if (command[cmd_len] == '\n' || command[cmd_len] == '\r') {
                command[cmd_len] = '\0'; // Null-terminate the command
                print("\n"); // Print newline
                break;
            } else { // Normal character input
                if (cmd_len >= MAX_COMMAND_LENGTH - 1) {
                    continue;
                }
                char single[2] = {command[cmd_len], '\0'};
                print(single);
            }

            cmd_len++;
        }

        // Execute command if it is not empty

        if (cmd_len == 0) {
            continue; // No command entered, prompt again
        } else {
            shell_exec(command);
        }
    }
}

void shell_exec(char* command) {
    if (shell_run_builtin(command)) {
        return; // If it was a built-in command, do not execute further
    }

    // If not a built-in command, execute as a program
    exec(command);
}

void shell_print_prompt() {
    print(current_directory);
    print("> ");
}