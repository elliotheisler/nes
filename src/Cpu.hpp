#pragma once
#include <cstdint>
#include "Mapper.hpp"
#include "CpuAndPpu.hpp"
#include "instruction_database.hpp"
using json = nlohmann::json;
class Cpu {
  public:
    enum class CpuFlag {
        fCarry,
        fZero,
        fInterruptDisable,
        fDecimal,
        // https://www.nesdev.org/wiki/Status_flags#The_B_flag
        fUnusedFlag1,
        fUnusedFlag2,
        fOverflow,
        fNegative
    };

    Cpu(); // TODO: Cartridge parameter
    // set cpu to power-on state
    void do_poweron();
    // perform state changes equivalent to hitting the NES reset button
    void do_reset();
    

    void clock();

  private:
    static const std::array<InstRecord, 256> inst_db;


    uint8_t A, Y, X, SP, SR;
    uint16_t PC;

    // TODO: Cartridge cartridge;
    // only the cpu communicates with the APU and the NES's 2kb of RAM
    // TODO: Apu apu
    uint8_t RAM[1 << 11];

    int counter;


    // number of cycles that the next instruction will take
    int cycles();
    // execute current instruction
    void exec();
    // load/store functions
    uint8_t load8(uint16_t addr);
    uint16_t load16(uint16_t addr);
    void store8(uint16_t addr, uint8_t data);
    void store16(uint16_t addr, uint16_t data);
    void* do_loadstore(MemAccess m);
    // flag functions
    bool get_flag(CpuFlag flag);
    void set_flag(CpuFlag flag, bool val);

};

