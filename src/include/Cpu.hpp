#pragma once
#include <cstdint>

#include "Cartridge.hpp"
#include "InstructionDatabase.hpp"
#include "Mapper.hpp"
#include "common.hpp"
#include "r16.hpp"

class Cpu {
    using json     = nlohmann::json;
    using AddrMode = Instruction::AddrMode;

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
    explicit Cpu( Cartridge &p_cartridge );
    explicit Cpu( uint8_t m_A, uint8_t m_Y, uint8_t m_X, uint8_t m_SP,
                  uint8_t m_SR, uint16_t m_PC );

    // perform state changes equivalent to CPU poweron
    void do_poweron();
    // perform state changes equivalent to hitting the NES reset button
    void do_reset();

    void clock();

    uint8_t A, Y, X, SP, SR;
    r16 PC;

   private:
    Cartridge &cartridge;
    // only the cpu communicates with the APU and the NES's 2kb of RAM
    // TODO: Apu apu
    uint8_t RAM[1 << 11];

    int counter{ 0 };

    // execute current instruction
    void exec();
    int get_cycles( uint8_t opcode );
    int get_num_bytes( uint8_t opcode );
    AddrMode get_mode( uint8_t opcode );
    uint8_t get_affected_flags( uint8_t opcode );
    // load/store
    std::optional<r16> get_effective_addr( AddrMode m );
    enum class PageWrap { kDoPageWrap, kNoPageWrap };

    r16 load16( r16 addr, PageWrap pw );
    uint8_t load8( uint16_t addr );
    void store8( uint16_t addr, uint8_t payload );  // TODO: pagewrap?

    template <AccessType A>
    uint8_t addr_access( uint16_t addr, uint8_t payload = 0 );

    // flag functions
    bool get_flag( CpuFlag flag );
    void set_flag( CpuFlag flag, bool val );

    int cycles_elapsed{ 0 };
    // logging
    std::string to_assembly( const uint8_t opcode );
    void log_nintendulator();
};
