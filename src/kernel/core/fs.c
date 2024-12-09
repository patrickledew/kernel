#include "fs.h"
#include "types.h"

// fat12_entry uint16_to_fat12(uint16_t in) {
//     fat12_entry entry = { 0 };
//     entry.byte1 = in & 0x0F;
//     entry.byte2 = in >> 8 & 0xFF;
//     entry.byte3 = in >> 16 & 0xFF;
//     return entry;
// }

// uint16_t fat12_to_uint16(fat12_entry in) {
//     uint16_t ret = 0x0000;
//     ret |= entry.byte1;
//     ret |= entry
//     return ret;
// }
