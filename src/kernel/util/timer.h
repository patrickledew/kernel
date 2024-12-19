#ifndef TIMER_H
#define TIMER_H
#include "types.h"
#include "core/interrupts/interrupts.h"
// This contains defines for programming the Programmable Interval Timer (PIT).
// This allows for accurate timekeeping and running tasks at different intervals.
/** Ports for interacting with PIT */
#define PIT_PORT_COUNT0         0x40
#define PIT_PORT_COUNT1         0x41
#define PIT_PORT_COUNT2         0x42
#define PIT_PORT_CONTROL        0x43

/** OCW for setting up PIT */
// OCW bit for binary counter format (binary or BCD)
#define PIT_OCW_BINCOUNT_BIN    0b0
#define PIT_OCW_BINCOUNT_BCD    0b1

// OCW bits for PIT mode
#define PIT_OCW_MODE_INT_COUNT  0b000 << 1 // Interrupt on Terminal Count
#define PIT_OCW_MODE_HW_ONESHOT 0b001 << 1 // Hardware Triggered One-Shot
#define PIT_OCW_MODE_RATE       0b010 << 1 // Rate generator
#define PIT_OCW_MODE_SQUAREWAVE 0b011 << 1 // Square wave generator
#define PIT_OCW_MODE_SW_STROBE  0b100 << 1 // Software Triggered Strobe
#define PIT_OCW_MODE_HW_STROBE  0b101 << 1 // Software Triggered Strobe

// OCW bits for reading/writing registers
// PIT has 16-bit registers but 8-bit data bus,
// so we must specify whether we are reading/loading MSB or LSB
#define PIT_OCW_RL_LATCH        0b00 << 4 // Latch current counter value to be read from IO port
#define PIT_OCW_RL_LSB          0b01 << 4 // Read or load least significant byte of counter
#define PIT_OCW_RL_MSB          0b10 << 4 // Read or load most significant byte of counter
#define PIT_OCW_RL_LSB_THEN_MSB 0b11 << 4 // Read/load LSB first, then MSB
// OCW bits for selecting counter to read/write
#define PIT_OCW_SELECT_COUNT0   0b00 << 6
#define PIT_OCW_SELECT_COUNT1   0b01 << 6
#define PIT_OCW_SELECT_COUNT2   0b10 << 6

/** Frequency of the PIT's clock */
#define PIT_FREQ_HZ 1193181 // Really 1193181.666..hz


/** Interval Table - used to keep track of what routines to run at which intervals */
#define INTERVAL_TABLE_ENTRIES 32

typedef struct {
    uint32_t interval; // The number of ms to wait before running the routine.
    uint32_t start_tick; // The tick during which the interval was created.
    void (*routine)(); // The routine to run.
} interval_desc;

void timer_pit_init(uint16_t freq);
void timer_isr(InterruptFrame* frame);
void timer_tick();
void timer_interval_run();
int timer_interval_set(uint32_t ms, void (*routine)());
int timer_interval_clear(int index);

#endif