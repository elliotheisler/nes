#include "Cartridge.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

void Cartridge::read_file(std::string&& path) {
    std::ifstream ifs{path, std::ios::binary};
    std::clog << "reading rom " << path << std::endl;
    char header[16];
    ifs.read(header, 16);
    if (strncmp(header, "NES\x1A", 4)) {
        throw std::runtime_error(
            "missing the \"NES\\x1A\" INES file signature");
    }

    // skip over the trainer, if present
    const bool trainer_present = header[6] & 0b100;
    std::clog << "512-byte trainer present?: " << trainer_present << '\n';
    if (trainer_present) {
        ifs.ignore(512);
    }

    // read prg_rom
    const int prg_rom_size = header[4] << 14;
    std::clog << "prg_rom_size: " << (prg_rom_size >> 10) << "KiB" << std::endl;
    prg_rom.resize(prg_rom_size);
    ifs.read(reinterpret_cast<char*>(prg_rom.data()), prg_rom_size);
    assert(ifs.gcount() == prg_rom_size);

    // read chr_rom
    const int chr_rom_size = header[5] << 13;
    std::clog << "chr_rom_size: " << (chr_rom_size >> 10) << "KiB" << std::endl;
    chr_rom.resize(chr_rom_size);
    ifs.read(reinterpret_cast<char*>(chr_rom.data()), chr_rom_size);
    assert(ifs.gcount() == chr_rom_size);

    // mapper number
    int mapper_no = (header[7] & 0b11110000) | ((header[6] & 0b11110000) >> 4);
    std::clog << "mapper number: " << mapper_no << std::endl;
}
