// Userspace shell program running on the kernel.

// Syscalls available to us:
// - exec: Execute a new process
// - exit: Exit the current process
// - yield: Yield the current process to allow other processes to run
// - malloc: Allocate memory from the process's heap
// - free: Free memory allocated from the process's heap
// - print: Print a string to the console

#ifndef SHELL_H
#define SHELL_H
#include <stddef.h>
#include <stdbool.h>
#include "klib.h"

void main();

void shell_init();

void shell_loop();
void shell_exec(char* command);
void shell_print_prompt();
void shell_print_help();