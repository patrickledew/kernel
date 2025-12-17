#ifndef BUILTINS_H
#define BUILTINS_H

// Returns true if the command was a built-in command and was run, false otherwise.
bool shell_run_builtin(char* command);
void shell_builtin_help();
void shell_builtin_exit();
void shell_builtin_cd(char* rel_path);
void shell_builtin_ls();
void shell_builtin_pwd();

#endif