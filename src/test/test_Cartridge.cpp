#include <cassert>

#include "Cartridge.hpp"

int main(int argc, char* argv[]) {
    // TODO: use libzip to unzip NES roms
    Cartridge c;
    c.read_file("test/kevtris_nestest/nestest.nes");
    c.read_file("roms/Donkey Kong (World) (Rev 1).nes");
}
