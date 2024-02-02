#include "Cpu.hpp"

#include <bits/ranges_algo.h>
#include <sys/types.h>

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "Cartridge.hpp"
#include "Disassemble.hpp"
#include "InstructionDatabase.hpp"

using nlohmann::json;
using Instruction::AddrMode;
using enum Instruction::AddrMode;
using std::tuple;

Cpu::Cpu( Cartridge& p_cartridge )
    : cartridge{ p_cartridge }, A{ 0 }, X{ 0 }, Y{ 0 } {};

void Cpu::clock() {
    // do exec() on first cycle ...
    if ( counter == 0 ) {
        last_cycles = exec();
    }
    counter = (counter + 1) % last_cycles;
    // ... and act like it actually completed after <last_cycles> cycles
    if ( counter == 0 ) {
        cycles_elapsed += last_cycles;
        log_nintendulator();
    }
}
static int exec_00() {

}
static int exec_01() {
}
static int exec_10() {
}

// TODO: account for page cross extra cycle in all instructions
int Cpu::exec() {
    using enum PageWrap;
    // https://llx.com/Neil/a2/opcodes.html
    const uint8_t  opcode         = load8( PC );
    int            cycles         = get_cycles( opcode );
    int            num_bytes      = get_num_bytes( opcode );
    const AddrMode mode           = get_mode( opcode );
    auto [effective_addr, page_crossed] = get_effective_addr( mode );
    cycles += page_crossed;

    // need a default value since this is a reference i guess
    uint8_t  arg1;
    uint8_t  arg2;
    uint8_t& dest = A;
    uint8_t  res;
    r16      ret_addr;
    // ***some instructions that don't really belong anywhere
    switch ( opcode ) {
            // -- JMP-like instructions
            // 2 JMP absolute and indirect
        case 0x4C:
        case 0x6C:
            PC = effective_addr;
            return cycles;
            // BIT insts
        case 0x24:
        case 0x2C:
            arg1 = A;
            res  = arg1 & load8( effective_addr );
            set_flag( fOverflow, res & fOverflow );
            set_flag( fNegative, res & 0x80 );
            set_flag( fZero, !res );
            goto INC_PC;
        case 0xEA: // NOP
            goto INC_PC;
    }


    // ***main lookup table
    switch ( opcode & 0b11 ) {
        case 0b01: // aaabbb01
            dest = A;
            arg1 = A;
            arg2 = load8( effective_addr );
            switch ( opcode >> 5 ) {
                case 0b000:  // ORA
                    dest = res = arg1 | arg2;
                    break;
                case 0b001:  // AND
                    dest = res = arg1 & arg2;
                    break;
                case 0b010:  // EOR
                    dest = res = arg1 ^ arg2;
                    break;
                case 0b011:  // ADC
                    dest = res = arg1 + arg2 + get_flag( fCarry );
                    break;
                    // case 0b100: // STA
                    //   store8(effective_addr, arg1); break;
                    //                 case 0b101:  // LDA
                    //                     dest = res = arg1 = arg2;
                    //                     break;
                case 0b110:  // CMP
                    res = arg1 - arg2;
                    break;
                case 0b111:  // SBC
                    dest = res = arg1 + ~arg2 + get_flag( fCarry );
                    break;
            }
            // all cc == 01 (except stores) affect N and Z the same:
            set_flag( fNegative, res & 0x80 );
            set_flag( fZero, !res );
            // ADC, SBC, and CMP: C flag
            switch ( opcode >> 5 ) {
                case 0b011:  // ADC
                case 0b110:  // CMP
                case 0b111:  // SBC
                    set_flag( fCarry, res < arg1 );
                    // ADC and SBC: V flag
                    // logic: for signed addition, "overflowing" beyond bounds
                    // of -128 and 127 is only possible if arg1 and arg2 have
                    // the same sign and the result has the opposite sign for
                    // subtraction, arg1 and arg2 must have different signs
                    if ( opcode >> 5 == 0b011 ) {
                        set_flag( fOverflow,
                                  0x80 & ~( arg1 ^ arg2 ) & ( arg1 ^ res ) );
                    } else if ( opcode >> 5 == 0b111 ) {
                        set_flag( fOverflow,
                                  0x80 & ( arg1 ^ arg2 ) & ( arg1 ^ res ) );
                    }
            }
            break;
        case 0b10: // aaabbb10
            arg1 = load8( effective_addr );
            switch ( opcode >> 5 ) {
                case 0b000:  // ASL
                    res = arg1 << 1;
                    store8( effective_addr, res );
                    break;
                case 0b001:  // ROL
                    res = arg1 << 1 | arg1 >> 7;
                    store8( effective_addr, res );
                    break;
                case 0b010:  // LSR
                    res = arg1 >> 1;
                    store8( effective_addr, res );
                    break;
                case 0b011:  // ROR
                    res = arg1 >> 1 | arg1 << 7;
                    store8( effective_addr, res );
                    break;
                    // case 0b100: // STX ( or TXA )
                    //               case 0b101: // LDX ( or TAX )
                    //                 res = arg1;
                case 0b110:  // DEC ( or DEX )
                    res = arg1 - 1;
                    store8( effective_addr, res );
                    break;
                case 0b111:  // INC
                    res = arg1 + 1;
                    store8( effective_addr, res );
                    break;
            }
            // all cc == 10 (except stores, and transfers) affect N and Z the
            // same:
            set_flag( fNegative, res & 0x80 );
            set_flag( fZero, !res );
            // shift instructions set the C flag to the leftmost/rightmost bit,
            // i.e the one shifted "out"
            switch ( opcode >> 5 ) {
                case 0b000:  // ASL
                case 0b001:  // ROL
                    set_flag( fCarry, arg1 & 0x80 );
                    break;
                case 0b010:  // LSR
                case 0b011:  // ROR
                    set_flag( fCarry, arg1 & 0x01 );
                    break;
            }
            break;
        case 0b00:                              // aabbb00
            switch ( opcode >> 2 & 0b000111 ) {  // aaa**b00
                case 0b110:                      // flag setters/getters
                    switch ( opcode >> 5 ) {
                        case 0b000:
                            set_flag( fCarry, false );
                            break;
                        case 0b001:
                            set_flag( fCarry, true );
                            break;
                        case 0b010:
                            set_flag( fInterruptDisable, false );
                            break;
                        case 0b011:
                            set_flag( fInterruptDisable, true );
                            break;
                        case 0b100:  // TYA
                            arg1 = Y;
                            dest = A;
                            res = dest = arg1;
                            break;
                        case 0b101:
                            set_flag( fOverflow, false );
                            break;
                        case 0b110:
                            set_flag( fDecimal, false );
                            break;
                        case 0b111:
                            set_flag( fDecimal, true );
                            break;
                    }
                    goto NO_LDST_00;
                case 0b100:  // branches
                    switch ( opcode >> 6 ) {
                        case 0b00:
                            if ( ( ( opcode >> 5 ) & 0b001 ) ==
                                 get_flag( fNegative ) ) {
                                PC = effective_addr;
                                return cycles + 1;
                            }
                            break;
                        case 0b01:
                            if ( ( ( opcode >> 5 ) & 0b001 ) ==
                                 get_flag( fOverflow ) ) {
                                PC = effective_addr;
                                return cycles + 1;
                            }
                            break;
                        case 0b10:
                            if ( ( ( opcode >> 5 ) & 0b001 ) ==
                                 get_flag( fCarry ) ) {
                                PC = effective_addr;
                                return cycles + 1;
                            }
                            break;
                        case 0b11:
                            if ( ( ( opcode >> 5 ) & 0b001 ) ==
                                 get_flag( fZero ) ) {
                                PC = effective_addr;
                                return cycles + 1;
                            }
                            break;
                    }
                    goto NO_LDST_00;
                case 0b000:
                    if ( !( opcode & 0x80 ) ) {  //  BRK, JSR, RTI, RTS
                        switch ( opcode ) {
                            // BRK
                            case 0x00:
                                interrupt<iBRK>();
                                return cycles;
                            // JSR
                            case 0x20:
                                ret_addr  = PC;
                                ret_addr += 2;
                                push( ret_addr.page );
                                push( ret_addr.index );
                                effective_addr = load16( PC + 1, kNoPageWrap );
                                PC             = effective_addr;
                                return cycles;
                            // -- RTS-like instructions
                            // RTI
                            case 0x40:
                                P                    = pop();
                                effective_addr.index = pop();
                                effective_addr.page  = pop();
                                PC                   = effective_addr;
                                return cycles;
                            // RTS
                            case 0x60:
                                effective_addr.index = pop();
                                effective_addr.page  = pop();
                                effective_addr++;
                                PC = effective_addr;
                                return cycles;
                        }
                    }
                    break;
                case 0b010:
                    if ( !( opcode & 0x80 ) ) {  // PHP/PLP, PHA/PLA
                        switch ( opcode >> 5 ) {
                            case 0b00:
                                push( P );
                                break;
                            case 0b01:
                                P = pop();
                                break;
                            case 0b10:
                                push( A );
                                break;
                            case 0b11:
                                res = A = pop();
                                set_flag( fNegative, res & 0x80 );
                                set_flag( fZero, !res );
                                break;
                        }
                    } else {  // DEY, TAY, INY, INX
                        switch ( opcode >> 5 ) {
                            case 0b00:
                                res = Y = Y - 1;
                                break;
                            case 0b01:
                                res = Y = A;
                                break;
                            case 0b10:
                                res = Y = Y + 1;
                                break;
                            case 0b11:
                                res = X = X + 1;
                                break;
                        }
                        set_flag( fNegative, res & 0x80 );
                        set_flag( fZero, !res );
                    }
                    goto NO_LDST_00;
            }
            // LD/ST
            switch (opcode >> 5) {
              case 0b100:
              case 0b101:
                goto LOAD_STORE;
            }
NO_LDST_00:
            // CPY, CPX, INY, INX: 11a0bb00
            if ( !( opcode & 0b00010000 ) ) {
                switch ( opcode >> 5 ) {
                    case 0b110:
                        arg1 = dest = Y;
                        break;
                    case 0b111:
                        arg1 = dest = X;
                        break;
                }
                switch ( opcode >> 2 & 0b11 ) {  // 11a01100
                        //                 case 0b10: // IN(Y|X) **already
                        //                 handled above!
                        //                   res = dest = arg1 + 1; break;
                    case 0b00:  // CP(Y|X)
                    case 0b01:
                    case 0b11:
                        arg2 = load8( effective_addr );
                        res  = arg1 - arg2;
                        break;
                }
                set_flag( fNegative, res & 0x80 );
                set_flag( fZero, !res );
                set_flag( fCarry, res < arg1 );
            }
            goto INC_PC;
        default: break;
//             throw std::logic_error( "unofficial opcode: cc == 11" );
    }

    // ***loads and stores
LOAD_STORE:
    switch ( opcode & 0b00000011 ) {
        case 0b00:
            arg1 = dest = Y;
            break;
        case 0b01:
            arg1 = dest = A;
            break;
        case 0b10:
            arg1 = dest = X;
            break;
    }

    // stores: have 8th bit set
    if ( ( opcode >> 5 ) == 0b100 ) {
        store8( effective_addr, arg1 );
    }
    // loads: have 8th and 6th bits set
    else if ( ( opcode >> 5 ) == 0b101 ) {
        res = dest = load8( effective_addr );
        set_flag( fNegative, res & 0x80 );
        set_flag( fZero, !res );
    }




INC_PC:
    PC = PC + num_bytes;
    return cycles;
}

AddrMode Cpu::get_mode( uint8_t opcode ) {
    return Instruction::TABLE[opcode].adr_mode;
}

int Cpu::get_cycles( uint8_t opcode ) {
    return Instruction::TABLE[opcode].cycles;
}

int Cpu::get_num_bytes( uint8_t opcode ) {
    return Instruction::TABLE[opcode].bytes;
}

uint8_t Cpu::get_affected_flags( uint8_t opcode ) {  // TODO
    return 0;
}

std::string Cpu::to_assembly( const uint8_t opcode ) {
    const InstRecord i_record = Instruction::TABLE[opcode];
    return i_record.name;
}

tuple<r16, Cpu::PageCross> Cpu::get_effective_addr( AddrMode m ) {
    using enum PageWrap;
    using enum PageCross;
    r16 addr;
// used in checking page boundary cross:
    PageCross page_crossed = cPageNotCrossed; 
    uint8_t old_page;
    switch ( m ) {
        case mZeroPage:
            addr = load8( PC + 1 );
            break;
        case mZeroPageX:
            addr        = load8( PC + 1 );
            addr.index += X;
            break;
        case mZeroPageY:
            addr        = load8( PC + 1 );
            addr.index += Y;
            break;

        case mAbsolute:
            addr = load16( PC + 1, kNoPageWrap );
            break;
        case mAbsoluteX:
            addr  = load16( PC + 1, kNoPageWrap );
            old_page = addr.page;
            addr += X;
            page_crossed = old_page == addr.page
                ? cPageNotCrossed
                : cPageCrossed;
            break;
        case mAbsoluteY:
            addr  = load16( PC + 1, kNoPageWrap );
            old_page = addr.page;
            addr += Y;
            page_crossed = old_page == addr.page
                ? cPageNotCrossed
                : cPageCrossed;
            break;

        case mIndirect:
            addr = load16( PC + 1, kDoPageWrap );
            addr = load16( addr, kDoPageWrap );
        case mIndirectX:
            addr        = load8( PC + 1 );
            addr.index += X;
            addr        = load16( addr, kDoPageWrap );
            break;
        case mIndirectY:
            addr  = load8( PC + 1 );
            addr  = load16( addr, kDoPageWrap );
            old_page = addr.page;
            addr += Y;
            page_crossed = old_page == addr.page
                ? cPageNotCrossed
                : cPageCrossed;
            break;

        case mImmediate:
            addr = PC + 1;
            break;
        case mRelative:
            addr  = PC + 2;
            old_page = addr.page;
            addr += static_cast<int>( load8( PC +1) );
            page_crossed = old_page == addr.page
                ? cPageNotCrossed
                : cPageCrossed;
            break;
        case mAccumulator:
        case mImplied:
            return tuple(0, page_crossed);  // TODO
    }
    return tuple(addr, page_crossed);
}

template <Cpu::IntType I>
void Cpu::interrupt() {
    using enum IntType;
    using enum PageWrap;
    using enum CpuFlag;
    constexpr static uint16_t INT_VECTORS[] = {
        [iNMI] = 0xFFFA, [iReset] = 0xFFFC, [iIRQ] = 0xFFFE, [iBRK] = 0xFFFE };

    switch ( I ) {
        case iReset:  // no writes on stack for resets
            SP -= 3;
            break;
        case iIRQ:
        case iNMI:
        case iBRK:
            r16 ret_addr = PC + 2;
            push( ret_addr.page );
            push( ret_addr.index );
            // BRK pushes B flag set to 1, IRQ and NMI set to 0
            push( I == iBRK ? P | fBFlag : P & ~fBFlag );
    }
    PC = load16( INT_VECTORS[I], kNoPageWrap );
    // in reality, this occurs between reading the low and high PC bytes
    set_flag( fInterruptDisable, true );
}

void Cpu::do_poweron() {
    // https://www.nesdev.org/wiki/CPU_power_up_state#At_power-up
    cycles_elapsed = 7; /* nestest.log starts with 7 cycles elapsed, so I assume
                           the poweron sequence is like BRK and INT */
    P = 0b00110100;
    A = X = Y = 0x00;
    SP        = 0b11111101;
    store8( 0x4017, 0 );  // frame irq enabled
    store8( 0x4015, 0 );  // all channels disabled
    for ( uint16_t addr = 0x4000; addr <= 0x4005; addr++ ) {
        store8( addr, 0 );
    }
    for ( uint16_t addr = 0x4010; addr <= 0x4013; addr++ ) {
        store8( addr, 0 );
    }
    // TODO:
    //     All 15 bits of noise channel LFSR = $0000[5]. The first time the
    //     LFSR is clocked from the all-0s state, it will shift in a 1. APU
    //     Frame Counter:
    //
    //         2A03E, G, various clones: APU Frame Counter reset.
    //         2A03letterless: APU frame counter powers up at a value
    //         equivalent to 15
    //
    //     Internal memory ($0000-$07FF) has unreliable startup state. Some
    //     machines may have consistent RAM contents at power-on, but others
    //     do not.
    //
    //         Emulators often implement a consistent RAM startup state
    //         (e.g. all $00 or $FF, or a particular pattern), and flash
    //         carts like the PowerPak may partially or fully initialize RAM
    //         before starting a program, so an NES programmer must be
    //         careful not to rely on the startup contents of RAM.
    log_nintendulator();
}

void Cpu::do_reset() {
    // https://www.nesdev.org/wiki/CPU_power_up_state#After_reset
    interrupt<iReset>();
    store8( 0x4015, 0 );
    // TODO:
    //     APU triangle phase is reset to 0 (i.e. outputs a value of 15, the
    //     first step of its waveform) APU DPCM output ANDed with 1 (upper 6
    //     bits cleared)
    //
    //         APU Frame Counter:
    //         2A03E, G, various clones: APU Frame Counter reset.
    //         2A03letterless: APU frame counter retains old value [6]
}

r16 Cpu::load16( r16 addr, PageWrap pw ) {
    using enum PageWrap;
    r16 ret;
    ret.index = load8( addr );
    switch ( pw ) {
        case kDoPageWrap:
            addr.index++;
            break;
        case kNoPageWrap:
            addr++;
    }
    ret.page = load8( addr );
    return ret;
}

// TODO: could add load/stores for zero page i.e. load8(addr.index);
uint8_t Cpu::load8( uint16_t addr ) { return addr_access<kLoad>( addr ); }

void Cpu::store8( uint16_t addr, uint8_t payload ) {
    addr_access<kStore>( addr, payload );
}

// push/pop
void Cpu::push( uint8_t payload ) {
    store8( 0x0100 | SP, payload );
    SP--;
}

uint8_t Cpu::pop() {
    SP++;
    return load8( 0x0100 | SP );
}

template <AccessType A>
uint8_t Cpu::addr_access( uint16_t addr, uint8_t payload ) {
    // https://www.nesdev.org/wiki/CPU_memory_map

    switch ( addr >> 13 ) {
        case 0b000:  // 2 KB internal RAM
            switch ( A ) {
                case kLoad:
                    return RAM[addr & 0b0000011111111111];
                case kStore:
                    RAM[addr & 0b0000011111111111] = payload;
                    return 0;
            }
        case 0b001:  // 8 NES PPU registers
            // actual_addr = addr & 0b0000000000000111;
            break;
        default:
            if ( addr > 0b0100000000011111 ) {
                // FINALLY: cartridge space
                return cartridge.cpu_access<A>( addr, payload );
            } else if ( ( addr & 0b11000 ) == 0b11000 ) {
                // SECOND; 8 bytes: "APU and I/O functionality that is
                // normally disabled." See CPU Test Mode
            } else {
                // FIRST 3 * 8 bytes: NES APU and I/O registers
            }
    }
    return 0;
}

// flags
bool Cpu::get_flag( CpuFlag flag ) { return P & static_cast<uint8_t>( flag ); }

void Cpu::set_flag( CpuFlag flag, bool val ) { P = val ? P | flag : P & ~flag; }

// =================logging
#include <format>

void Cpu::log_nintendulator() {
    using namespace Disassemble;
    const uint8_t     opcode = load8( PC );
    const uint8_t     arg1   = load8( PC + 1 );
    const uint8_t     arg2   = load8( PC + 2 );
    const ParsedInst  cur_inst{ PC, opcode, arg1, arg2 };
    const InstRecord& record = cur_inst.record;

    const std::string arg1str{
        record.bytes < 2
            ? ""
            : std::vformat( "{:02X}", std::make_format_args( arg1 ) ) };

    const std::string arg2str{
        record.bytes < 3
            ? ""
            : std::vformat( "{:02X}", std::make_format_args( arg2 ) ) };

    std::string msg = std::vformat(
             "{:02X}{:02X}  {:02X} {:2} {:2}  {:32}A:{:02X} X:{:02X} Y:{:02X} P:{:02X} SP:{:02X}"
             " PPU:{:8}"  // PPU fields placeholder
             "CYC:{}", std::make_format_args(
             PC.page, PC.index, opcode, arg1str.c_str(), arg2str.c_str(),
             cur_inst.to_string().c_str(), A, X, Y, P, SP, "",
             cycles_elapsed ));
    std::cout << msg << std::endl;
}
