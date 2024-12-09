#include "timer.h"
#include "portio.h"
#include "logging.h"
#include "core/interrupts.h"

// Minimum frequency is ~18.2 hz, since the max value for count registers is 65535.
// PIT_FREQ_HZ / 65535 = 18.2
void init_pit(uint16_t freq) {
    uint16_t initial_count = PIT_FREQ_HZ / freq; // Run PIT at [freq]hz
    uint8_t ocw = PIT_OCW_BINCOUNT_BIN
                | PIT_OCW_MODE_RATE
                | PIT_OCW_RL_LSB_THEN_MSB
                | PIT_OCW_SELECT_COUNT0;

    outb(PIT_PORT_CONTROL, ocw); // Send control word
    outb(PIT_PORT_COUNT0, (initial_count & 0xFF)) // Load LSB
    outb(PIT_PORT_COUNT0, ((initial_count >> 8) & 0xFF)) // Load MSB

    // Now register the ISR for the timer IRQ
    ADD_ISR(0x20, timer_isr); // Initialize keyboard IRQ
}

// The number of ms the kernel has been running for.
__attribute__((interrupt))
void timer_isr(interrupt_frame* frame) {
    uint8_t c = get_color();
    timer_tick();
    pic_eoi();
}


uint32_t tick = 0;
void timer_tick() {
    tick++;
    handle_intervals();
}

interval_desc interval_table[INTERVAL_TABLE_ENTRIES] = { 0 };

void handle_intervals() {
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
int set_interval(uint32_t ms, void (*routine)()) {
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
int clear_interval(int index) {
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