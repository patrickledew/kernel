/*
 * types.h
 *
 * This header file defines standard data types and boolean constants used throughout
 * the kernel. It provides type aliases for fixed-width integers and defines TRUE
 * and FALSE macros for boolean operations, ensuring consistent data type usage
 * across the system.
 */

#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdbool.h>

typedef char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

#define FALSE false
#define TRUE true

#endif