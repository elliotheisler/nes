#include <cstdint>
#include "Mapper.hpp"
#include "Cpu.hpp"
#include "common.hpp"

void Cpu::clock() {
        counter++;
        if (cycles() == counter) {
            exec();
            counter = 0;
        }
}

void Cpu::do_poweron() {
    // https://www.nesdev.org/wiki/CPU_power_up_state#At_power-up
    SR = 0b00110100;
    A = X = Y = 0x00;
    SP = 0b11111101;
    store8(0x4017, 0); // frame irq enabled
    store8(0x4015, 0); // all channels disabled
    for (uint16_t addr = 0x4000; addr <= 0x4005; addr++) {
        store8(addr, 0);
    }
    for (uint16_t addr = 0x4010; addr <= 0x4013; addr++) {
        store8(addr, 0);
    }
    // TODO:
//     All 15 bits of noise channel LFSR = $0000[5]. The first time the LFSR is clocked from the all-0s state, it will shift in a 1.
//     APU Frame Counter:
// 
//         2A03E, G, various clones: APU Frame Counter reset.
//         2A03letterless: APU frame counter powers up at a value equivalent to 15
// 
//     Internal memory ($0000-$07FF) has unreliable startup state. Some machines may have consistent RAM contents at power-on, but others do not.
// 
//         Emulators often implement a consistent RAM startup state (e.g. all $00 or $FF, or a particular pattern), and flash carts like the PowerPak may partially or fully initialize RAM before starting a program, so an NES programmer must be careful not to rely on the startup contents of RAM.
}

void Cpu::do_reset() {
    // https://www.nesdev.org/wiki/CPU_power_up_state#After_reset
    using enum Cpu::CpuFlag;
    SP -= 3;
    set_flag(fInterruptDisable, true);
    store8(0x4015, 0);
    // TODO:
//     APU triangle phase is reset to 0 (i.e. outputs a value of 15, the first step of its waveform)
//     APU DPCM output ANDed with 1 (upper 6 bits cleared)
// 
//         APU Frame Counter:
//         2A03E, G, various clones: APU Frame Counter reset.
//         2A03letterless: APU frame counter retains old value [6]
}

uint8_t* Cpu::apply_addr8(uint16_t addr) {
    // https://www.nesdev.org/wiki/CPU_memory_map
    uint16_t actual_addr;
    if (check_bits("000x_xxxx_xxxx_xxxx", addr)) {
        // 2KB internal ram
    }
    else if (check_bits("001x_xxxx_xxxx_xxxx", addr)) {
        // 8 NES PPU registers
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
// const json Cpu::inst_db{ read_json("instructions.json") };
