#include <cstdint>
#include "Mapper.hpp"
#include "json.hpp"
#include "common.hpp"
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
    inline static const json inst_db{ read_json(INST_JSON_PATH) };
    uint8_t A, Y, X, SP, SR;
    uint16_t PC;

    // TODO: Cartridge cartridge;
    // only the cpu communicates with the APU and the NES's 2kb of RAM
    // TODO: Apu apu
    uint8_t ram[1 << 11];

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
    uint16_t* apply_addr16(uint16_t addr);
    uint8_t* apply_addr8(uint16_t addr);
    // flag functions
    bool get_flag(CpuFlag flag);
    void set_flag(CpuFlag flag, bool val);
};
