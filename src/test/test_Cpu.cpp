#include <iostream>

#include "Cartridge.hpp"
#include "Cpu.hpp"

int main() {
    using enum Cpu::CpuFlag;
    Cartridge cartridge;
    cartridge.read_file( "test/kevtris_nestest/nestest.nes" );
    Cpu cpu{ cartridge };
    // to be consistent with startup state of nestest.nes
    cpu.PC = 0xC000;
    cpu.do_poweron();
    // to be consistent with startup state of nestest.nes
    cpu.set_flag( fBFlag, false );

    for ( int i = 0; i < 1e4; i++ ) {
        cpu.clock();
    }
}
