/*
 * panic.h
 *
 * This file provides panic functionality for the kernel, allowing it to
 * halt execution and display an error message when an unrecoverable error
 * occurs. The panic function is designed to be called in situations where
 * the system cannot safely continue operation.
 */

#ifndef PANIC_H
#define PANIC_H

__attribute__((noreturn))
void panic(char* msg);

#endif