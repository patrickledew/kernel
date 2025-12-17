
bool shell_run_builtin(char* command) {
    // Check for built-in commands
    if (strcmp(command, "help", 0) == 0) {
        shell_builtin_help();
        return true;
    } else if (strcmp(command, "exit", 0) == 0) {
        shell_builtin_exit();
        return true;
    }

    return false; // Not a built-in command
}

void shell_builtin_help() {
    print("+-----------------------------------------------------+\n");
    print("| Ricky's Shell - v0.02                               |\n");
    print("| Available commands:                                 |\n");
    print("| - help: Show this help message                      |\n");
    print("| - exit: Exit the shell                              |\n");
    print("| - cd <dir>: Change directory                        |\n");
    print("| - pwd: Print current directory                      |\n");
    print("| - exit: Exit the shell                              |\n");
    print("| - <filename>: Execute a file as a process           |\n");
    print("+-----------------------------------------------------+\n");
}

void shell_builtin_exit() {
    print("Exiting shell...\n");
    exit(); // Exit the shell process
}

