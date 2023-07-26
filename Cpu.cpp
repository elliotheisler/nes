#include <cstdint>
#include "Mapper.hpp"
#include "Cpu.hpp"
#include "common.hpp"

Cpu::Cpu(Mapper new_mapper)
      : mapper { new_mapper }
    {}

uint8_t* Cpu::apply_addr(uint16_t addr) {
    // https://www.nesdev.org/wiki/CPU_memory_map
    uint16_t actual_addr;
    if (check_bits("000x_xxxx_xxxx_xxxx", addr)) {
    }
    else if (check_bits("001x_xxxx_xxxx_xxxx", addr)) {
    } 
    else {
            if (addr >= 0b0100000000100000) {
                    // cartridge space
            } else if (check_bits("xxxx_xxxx_xxx1_1xxx", addr)) {
                    // APU and I/O functionality that is normally disabled. See CPU Test Mode. 
            } else {
                    // NES APU and I/O registers
            }
    }
// faster implementation not using check_bits()
//     switch (addr >> 13) {
//         case 0b000: // 2 KB internal RAM
//             actual_addr = addr & 0b0000011111111111;
//             break;
//         case 0b001: // 8 NES PPU registers
//             actual_addr = addr & 0b0000000000000111;
//             break;
//         default: 
//             if (addr >= 0b0100000000100000) {
//                     // cartridge space
//             } else if (select_bitrange(addr, 0b11000) == 0b11) {
//                     // APU and I/O functionality that is normally disabled. See CPU Test Mode. 
//             } else {
//                     // NES APU and I/O registers
//             }
//     }
}
