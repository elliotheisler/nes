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
    enum CpuFlag {
        fCarry            = 1 << 0,
        fZero             = 1 << 1,
        fInterruptDisable = 1 << 2,
        fDecimal          = 1 << 3,
        // https://www.nesdev.org/wiki/Status_flags#The_B_flag
        fBFlag       = 1 << 4,
        fUnusedFlag2 = 1 << 5,
        fOverflow    = 1 << 6,
        fNegative    = 1 << 7,
    };

    // flag functions
    bool get_flag( CpuFlag flag );
    void set_flag( CpuFlag flag, bool val );
    // constructors
    explicit Cpu( Cartridge& p_cartridge );
    explicit Cpu( uint8_t m_A, uint8_t m_Y, uint8_t m_X, uint8_t m_SP,
                  uint8_t m_SR, uint16_t m_PC );

    // perform state changes equivalent to CPU poweron
    void do_poweron();
    // perform state changes equivalent to hitting the NES reset button
    void do_reset();


    void clock();

    uint8_t A, Y, X, SP, P;
    r16     PC;

   private:
    enum IntType { iIRQ, iNMI, iBRK, iReset };

    template <IntType I>
    void interrupt();

    Cartridge& cartridge;
    // only the cpu communicates with the APU and the NES's 2kb of RAM
    // TODO: Apu apu
    uint8_t RAM[1 << 11];


    // execute current instruction
    int     exec(); // returns number of cycles taken
    int exec_00( const InstRecord &r );
    int exec_01( const InstRecord &r );
    int exec_10( const InstRecord &r );
    int      get_cycles( uint8_t opcode );
    int      get_num_bytes( uint8_t opcode );
    AddrMode get_mode( uint8_t opcode );
    uint8_t  get_affected_flags( uint8_t opcode );
    // load/store
    enum class PageWrap { kDoPageWrap, kNoPageWrap };
    enum PageCross { cPageCrossed = 1, cPageNotCrossed = 0};
    std::tuple<r16, PageCross>     get_effective_addr( AddrMode m , uint8_t opcode);
    r16     load16( r16 addr, PageWrap pw );
    uint8_t load8( uint16_t addr );
    void    store8( uint16_t addr, uint8_t payload );  // TODO: pagewrap?
    // push/pop
    void    push( uint8_t payload );
    uint8_t pop();

    template <AccessType A>
    uint8_t addr_access( uint16_t addr, uint8_t payload = 0 );

    // logging
    std::string to_assembly( const uint8_t opcode );
    void        log_nintendulator();
    // state for clock() and logging
    int counter{ 0 };
    int cycles_elapsed{ 0 };
    int last_cycles{ 0 };
};
