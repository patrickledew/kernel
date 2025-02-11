#include "timer.h"
#include "core/portio/portio.h"
#include "logging.h"
#include "core/interrupts/interrupts.h"
#include "util/assert.h"

// Minimum frequency is ~18.2 hz, since the max value for count registers is 65535.
// PIT_FREQ_HZ / 65535 = 18.2
void timer_init(uint16_t freq) {
    log_info("timer_init: initializing hardware timer.");
    uint16_t initial_count = PIT_FREQ_HZ / freq; // Run PIT at [freq]hz
    uint8_t ocw = PIT_OCW_BINCOUNT_BIN
                | PIT_OCW_MODE_RATE
                | PIT_OCW_RL_LSB_THEN_MSB
                | PIT_OCW_SELECT_COUNT0;

    outb(PIT_PORT_CONTROL, ocw); // Send control word
    outb(PIT_PORT_COUNT0, (initial_count & 0xFF)); // Load LSB
    outb(PIT_PORT_COUNT0, ((initial_count >> 8) & 0xFF)); // Load MSB

    // Now register the ISR for the timer IRQ
    REG_ISR(0x20, timer_isr); // Initialize keyboard IRQ
}

__attribute__((interrupt))
void timer_isr(InterruptFrame* frame) {
    timer_tick();
    int_pic_send_eoi();
}


uint32_t tick = 0;
void timer_tick() {
    tick++;
    timer_interval_run();
}

interval_desc interval_table[INTERVAL_TABLE_ENTRIES] = { 0 };

void timer_interval_run() {
    for (int i = 0; i < INTERVAL_TABLE_ENTRIES; i++) {
        interval_desc interval = interval_table[i];
        if (interval.interval == 0) continue;

        if ((tick - interval.start_tick) % interval.interval == 0) {
            // Run interval routine
            interval.routine();
        }
    }
}

// Returns an index to the created entry in the interval table, or -1 if the interval table is full.
int timer_interval_set(uint32_t ms, void (*routine)()) {
    for (int i = 0; i < INTERVAL_TABLE_ENTRIES; i++) {
        if (interval_table[i].interval == 0) { // If interval empty, create one
            interval_table[i].interval = ms;
            interval_table[i].start_tick = tick;
            interval_table[i].routine = routine;

            return i;
        }
        i++;
    }
    return -1;
}

// Returns 1 if interval successfully cleared, 0 if the interval does not exist.
int timer_interval_clear(int index) {
    if (index < 0 || index >= INTERVAL_TABLE_ENTRIES) {
        return 0;
    }
    
    if (interval_table[index].interval == 0) {
        return 0;
    }
    
    interval_table[index].interval = 0;
    interval_table[index].start_tick = 0;
    interval_table[index].routine = 0;
    return 1;
}