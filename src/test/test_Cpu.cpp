#include <iostream>

#include "Cartridge.hpp"
#include "Cpu.hpp"

int main() {
    Cartridge cartridge;
    cartridge.read_file("test/kevtris_nestest/nestest.nes");
    Cpu cpu{cartridge};
    cpu.PC = 0xC000;
    cpu.do_poweron();

    for (int i = 0; i < 1e2; i++) {
        cpu.clock();
    }
}
