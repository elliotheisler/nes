#pragma once
#include <cstdint>

#include "Cartridge.hpp"
#include "Mapper.hpp"
#include "common.hpp"
#include "instruction_database.hpp"

using json = nlohmann::json;
class Cpu {
   public:
    /*! class for 16-bit registers in the CPU. encapsulates the
     * 8-bit page wraparound when adding address offsets, as seen in
     * some of the addressing modes.
     */
    class r16 {
       public:
        uint8_t page;
        uint8_t index;
        r16(uint16_t i);
        operator uint16_t() const;

        r16 operator+(uint8_t other);
        r16 operator+(int other);

        r16 operator++(int);

        r16 operator=(uint8_t val);
        r16 operator+=(uint8_t val);
        /*
* types of atomic address accesses that occur
* as part of the various addressing modes
u8  -> u8  - (8bit address with 8bit result ex: zero page)
u16 -> u8  - etc...
u8  -> u16 w ...
u16 -> u16 w (16bit address with 16bit result, with pagewrap)
u16 -> u16 - (16bit address with 16bit result, no pagewrap)
*/
    };

   public:
    enum class CpuFlag {
        fCarry            = 1 << 0,
        fZero             = 1 << 1,
        fInterruptDisable = 1 << 2,
        fDecimal          = 1 << 3,
        // https://www.nesdev.org/wiki/Status_flags#The_B_flag
        fUnusedFlag1 = 1 << 4,
        fUnusedFlag2 = 1 << 5,
        fOverflow    = 1 << 6,
        fNegative    = 1 << 7,
    };

    Cpu(Cartridge& p_cartridge);
    Cpu(uint8_t m_A, uint8_t m_Y, uint8_t m_X, uint8_t m_SP, uint8_t m_SR,
        uint16_t m_PC);

    // set cpu to power-on state
    void do_poweron();
    // perform state changes equivalent to hitting the NES reset button
    void do_reset();

    void clock();

    uint8_t A, Y, X, SP, SR;
    r16 PC = 0x0000;

   private:
    static const std::array<InstRecord, 256> inst_db;

    Cartridge& cartridge;
    // only the cpu communicates with the APU and the NES's 2kb of RAM
    // TODO: Apu apu
    uint8_t RAM[1 << 11];

    int counter;

    // execute current instruction
    void exec();
    int get_cycles(uint8_t opcode);
    int get_num_bytes(uint8_t opcode);
    AddrMode get_mode(uint8_t opcode);
    uint8_t get_affected_flags(uint8_t opcode);
    // load/store
    std::optional<r16> get_effective_addr(AddrMode m);
    enum class PageWrap { kDoPageWrap, kNoPageWrap };

    r16 load16(r16 addr, PageWrap pw);
    uint8_t load8(uint16_t addr);
    void store8(uint16_t addr, uint8_t payload);  // TODO: pagewrap?

    template <AccessType A>
    uint8_t addr_access(uint16_t addr, uint8_t payload);

    // flag functions
    bool get_flag(CpuFlag flag);
    void set_flag(CpuFlag flag, bool val);

    int cycles_elapsed;
    // logging
    std::string to_assembly(const uint8_t opcode);
    void log_nintendulator();
};
