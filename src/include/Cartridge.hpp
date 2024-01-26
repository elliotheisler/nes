#pragma once
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "common.hpp"
using enum AccessType;

class Cartridge {
   public:
    void read_file(std::string&& path);

    template <AccessType A>
    uint8_t cpu_access(uint16_t addr, uint8_t payload) {
        // https://www.nesdev.org/wiki/NROM
        if ((addr >> 13) < 0b100) {
            // Family basic: 2kb prg ram, tiled 4 times to fill 8kb window
            // prg_ram[addr & 0x07FF]
            throw std::logic_error(
                "NROM prg ram addressing not yet implemented");
        } else {
            // 16kb or 32kb of prg_rom
            switch (A) {
                case kLoad:
                    return prg_rom[addr % prg_rom.size()];
                    break;
                case kStore:
                    prg_rom[addr % prg_rom.size()] = payload;
                    return 0;
            }
        }
    }

   private:
    std::vector<uint8_t> prg_rom;
    std::vector<uint8_t> chr_rom;
};
