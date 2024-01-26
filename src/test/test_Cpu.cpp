#include <iostream>

#include "Cartridge.hpp"
#include "Cpu.hpp"

int main() {
    Cartridge cartridge;
    Cpu cpu{cartridge};
    cartridge.read_file("test/kevtris_nestest/nestest.nes");
    cpu.PC = Cpu::r16{0XC000};

    for (int i = 0; i < 1e2; i++) {
        cpu.clock();
    }
}
