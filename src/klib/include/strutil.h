/*
 * strutil.h
 *
 * A collection of utility functions for manipulating and comparing strings.
 * Provides basic string operations such as length calculation, copying,
 * splitting, and comparison for null-terminated strings. These functions
 * are essential for string handling in the kernel.
 */

#ifndef STRUTIL_H
#define STRUTIL_H

// Gets the length of a null-terminated string.
unsigned int strlen(char* str);

void strcpy(char* src, char* dest, unsigned int count);

// Gets the part of the string before the first occurrence of `c` after `skip` and copies it into `dest`.
// Returns the index of the character after the first occurrence of `c`, or -1 if there are no more instances of `c` in the string.
// If returning -1, `dest` is populated with the entirety of `str`.
int strsplit(char* str, char c, char* dest, unsigned int skip);

// Returns 0 if the two null-terminated strings have equal content.
// Returns 1 if:
// - The ascii value of the first differing character in `first` is greater than in `second`.
// - All characters in `second` match those in `first`, but `first` has more characters.
// Returns -1 if:
// - The ascii value of the first different character in `second` is greater than in `first`.
// - All characters in `first` match those in `second`, but `second` has more characters.
// Additionally, if `count` is greater than zero, only a max of `count` characters will be compared.
int strcmp(char* first, char* second, unsigned int count);

char* strcat(char* dest, char* source);


#endif