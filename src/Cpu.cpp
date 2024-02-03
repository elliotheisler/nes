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

using Instruction::AddrMode;
using nlohmann::json;
using enum Instruction::AddrMode;
using std::tuple;

Cpu::Cpu( Cartridge& p_cartridge )
    : cartridge{ p_cartridge }, A{ 0 }, X{ 0 }, Y{ 0 } {};

void Cpu::clock() {
    // do exec() on first cycle ...
    if ( counter == 0 ) {
        last_cycles = exec();
    }
    counter = ( counter + 1 ) % last_cycles;
    // ... and act like it actually completed after <last_cycles> cycles
    if ( counter == 0 ) {
        cycles_elapsed += last_cycles;
        log_nintendulator();
    }
}

class IllegalOpcodeError: virtual public std::exception {
    private:
        const std::string &message;
    public:
        IllegalOpcodeError(const std::string &msg) : message{msg} {}
        virtual const char* what() const throw () {
            return message.c_str();
        }
};

int Cpu::exec_00( const InstRecord& r ) {
    using enum PageWrap;
    constexpr CpuFlag BRANCH_FLAGS[] = { fNegative, fOverflow, fCarry, fZero };
    uint8_t           res, arg;
    r16               ret_addr;
    auto [effective_addr, page_crossed] = get_effective_addr( r.adr_mode );
    switch ( r.opcode >> 2 & 0b000111 ) {  // aaa**000
        case 0b110:                        // flag setters/getters
            PC++;
            switch ( r.opcode >> 5 ) {
                case 0b000:
                    set_flag( fCarry, false );
                    return 2;
                case 0b001:
                    set_flag( fCarry, true );
                    return 2;
                case 0b010:
                    set_flag( fInterruptDisable, false );
                    return 2;
                case 0b011:
                    set_flag( fInterruptDisable, true );
                    return 2;
                case 0b100:  // TYA
                    res = A = Y;
                    set_flag( fNegative, res & 0x80 );
                    set_flag( fZero, !res );
                    return 2;
                case 0b101:
                    set_flag( fOverflow, false );
                    return 2;
                case 0b110:
                    set_flag( fDecimal, false );
                    return 2;
                case 0b111:
                    set_flag( fDecimal, true );
                    return 2;
            }
        case 0b100:  // branches
            if ( ( r.opcode >> 5 & 0b001 ) ==
                 get_flag( BRANCH_FLAGS[r.opcode >> 6] ) ) {
                PC = effective_addr;
                return 2 + page_crossed + 1;
            } else {
                PC += 2;
                return 2;
            }
        case 0b000:  // BRK/JSR/RTI/RTS/***/CPY/CPX
            switch ( r.opcode >> 5 ) {
                case 0b000:  // BRK
                    interrupt<iBRK>();
                    return 7;
                case 0b001:  // JSR
                    ret_addr = PC + 2;
                    push( ret_addr.page );
                    push( ret_addr.index );
                    effective_addr = load16( PC + 1, kNoPageWrap );
                    PC             = effective_addr;
                    return 6;
                case 0b010:  // RTI
                    P                    = pop();
                    effective_addr.index = pop();
                    effective_addr.page  = pop();
                    PC                   = effective_addr;
                    return 6;
                case 0b011:  // RTS
                    effective_addr.index = pop();
                    effective_addr.page  = pop();
                    effective_addr++;
                    PC = effective_addr;
                    return 6;
                case 0b100:
                    throw IllegalOpcodeError("0x80");
                case 0b101:  // LDY imm
                    res = Y = load8( effective_addr );
                    break;
                case 0b110:  // CPY
                    arg = load8( effective_addr );
                    res = Y - arg;
                    set_flag( fCarry, arg <= Y); 
                    break;
                case 0b111:  // CPX
                    arg = load8( effective_addr );
                    res = X - arg;
                    set_flag( fCarry, arg <= X);
                    break;
            }
            set_flag( fNegative, res & 0x80 );
            set_flag( fZero, !res );
            PC += r.bytes;
            return r.cycles + page_crossed;
        case 0b010:  // push/pull/DEY/TAY/INY/INX
            switch ( r.opcode >> 5 ) {
                case 0b000:  // PHP
                    push( P | fBFlag ); // PHP, like BRK, pushes P with bit 4 == true
                    PC++;
                    return 3;
                case 0b001:  // PLP
                    P = pop();
                    PC++;
                    return 4;
                case 0b010:  // PHA
                    push( A );
                    PC++;
                    return 3;
                case 0b011:  // PLA
                    res = A = pop();
                    break;
                case 0b100:  // DEY
                    res = --Y;
                    break;
                case 0b101:  // TAY
                    res = Y = A;
                    break;
                case 0b110:  // INY
                    res = ++Y;
                    break;
                case 0b111:  // INX
                    res = ++X;
                    break;
            }
            set_flag( fNegative, res & 0x80 );
            set_flag( fZero, !res );
            PC += r.bytes;
            return r.cycles;
    }
    // note that this doesn't check all illegal opcodes
    switch ( r.opcode >> 5 ) {
        case 0b000:
            throw IllegalOpcodeError("000bb100");
        case 0b001:  // BIT
            arg = load8( effective_addr );
            res = A & arg;
            set_flag( fOverflow, arg & 1 << 6 );
            // BIT checks bit 7 of the
            // *argument*, not the result of
            // the bittest
            set_flag( fNegative, arg & 1 << 7 ); 
            set_flag( fZero, !res );
            PC += r.bytes;
            return r.cycles; // no page cross possible
        case 0b010:  // JMP abs
        case 0b011:  // JMP ind
            PC = effective_addr;
            return r.cycles;
        case 0b100:  // STY
            store8( effective_addr, Y );
            PC += r.bytes;
            return r.cycles;
        case 0b101:  // LDY: note - LDY absolute,X is the only cc == 00
                     // instruction that may cause an extra page boundary cycle
            res = Y = load8( effective_addr );
            break;
        case 0b110:  // CPY
            res = Y - load8( effective_addr );
            set_flag( fCarry, res < Y );
            break;
        case 0b111:  // CPX
            res = X - load8( effective_addr );
            set_flag( fCarry, res < X );
            break;
    }
    set_flag( fNegative, res & 0x80 );
    set_flag( fZero, !res );
    PC += r.bytes;
    return r.cycles + page_crossed;
}

int Cpu::exec_01( const InstRecord& r ) {
    auto [effective_addr, page_crossed] = get_effective_addr( r.adr_mode );
    uint8_t res;
    uint8_t arg2;
    switch ( r.opcode >> 5 ) {
        case 0b000:  // ORA
            arg2 = load8( effective_addr );
            A = res = A | arg2;
            break;
        case 0b001:  // AND
            arg2 = load8( effective_addr );
            A = res = A & arg2;
            break;
        case 0b010:  // EOR
            arg2 = load8( effective_addr );
            A = res = A ^ arg2;
            break;
        case 0b011:  // ADC
            arg2 = load8( effective_addr );
            res = A + arg2 + get_flag( fCarry );
            set_flag( fCarry, res < A );
            set_flag( fOverflow, 0x80 & ~( A ^ arg2 ) & ( A ^ res ) );
            A = res;
            break;
        case 0b100:  // STA
            store8( effective_addr, A );
            PC += r.bytes;
            return r.cycles + page_crossed;
        case 0b101:  // LDA
            arg2 = load8( effective_addr );
            A = res = arg2;
            break;
        case 0b110:  // CMP
            arg2 = load8( effective_addr );
            res  = A - arg2;
            // different carry flag logic from ADC and SBC:
            set_flag( fCarry, arg2 <= A);
            break;
        case 0b111:  // SBC
            arg2 = load8( effective_addr );
            res = A + ~arg2 + get_flag( fCarry );
            set_flag( fCarry, res < A );
            set_flag( fOverflow, 0x80 & ( A ^ arg2 ) & ( A ^ res ) );
            A = res;
            break;
    }
    // all cc == 01 (except stores) affect N and Z the same:
    set_flag( fNegative, res & 0x80 );
    set_flag( fZero, !res );
    PC += r.bytes;
    return r.cycles + page_crossed;
}

int Cpu::exec_10( const InstRecord& r ) {
    uint8_t res, arg;
    auto [effective_addr, page_crossed] = get_effective_addr( r.adr_mode );
    switch ( r.opcode >> 5 ) {
        case 0b000:  // ASL
            if (r.adr_mode == mAccumulator) {
                arg = A;
                A = res = arg << 1;
            } else {
                arg = load8( effective_addr );
                res = arg << 1;
                store8( effective_addr, res );
            }
            set_flag( fCarry, arg & 0x80 );
            break;
        case 0b001:  // ROL
            if (r.adr_mode == mAccumulator) {
                arg = A;
                A = res = arg << 1 | get_flag(fCarry);
            } else {
                arg = load8( effective_addr );
                res = arg << 1 | get_flag(fCarry);
                store8( effective_addr, res );
            }
            set_flag( fCarry, arg & 0x80 );
            break;
        case 0b010:  // LSR
            if (r.adr_mode == mAccumulator) {
                arg = A;
                A = res = arg >> 1;
            } else {
                arg = load8( effective_addr );
                res = arg >> 1;
                store8( effective_addr, res );
            }
            set_flag( fCarry, arg & 0x01 );
            break;
        case 0b011:  // ROR
            if (r.adr_mode == mAccumulator) {
                arg = A;
                A = res = arg >> 1 | get_flag(fCarry) << 7;
            } else {
                arg = load8( effective_addr );
                res = arg >> 1 | get_flag(fCarry) << 7;
                store8( effective_addr, res );
            }
            set_flag( fCarry, arg & 0x01 );
            break;
        case 0b100:  // STX ( or TX(A|S) )
            if ( ( r.opcode & 0b00011100 ) == 0b00001000 ) {  // TXA
                res = A = X;
            } else if ( ( r.opcode & 0b00011100 ) == 0b00011000 ) {  // TXS
                res = SP = X;
                PC++;
                return 2;  // TXS, unlike every other
                           // transfer instruction,
                           // affects no flags.
            } else {       // STX
                store8( effective_addr, X );
                PC += r.bytes;
                return r.cycles + page_crossed;  // stores affect no flags
            }
            break;
        case 0b101:  // LDX ( or T(A|S)X )
            if ( ( r.opcode & 0b00011100 ) == 0b00001000 ) {  // TAX
                res = X = A;
            } else if ( ( r.opcode & 0b00011100 ) == 0b00011000 ) {  // TSX
                res = X = SP;
            } else {  // LDX
                arg = load8( effective_addr );
                res = X = arg;
            }
            break;
        case 0b110:  // DEC ( or DEX )
            if (r.opcode == 0xCA) {
                res = --X;
            } else {
                arg = load8( effective_addr );
                res = arg - 1;
                store8( effective_addr, res );
            }
            break;
        case 0b111:  // INC ( or NOP )
            if ( r.opcode == 0xEA ) {
                PC++;
                return 2;
            }
            arg = load8( effective_addr );
            res = arg + 1;
            store8( effective_addr, res );
            break;
    }
    // all cc == 10 (except stores and TXS) affect N and Z the
    // same:
    set_flag( fNegative, res & 0x80 );
    set_flag( fZero, !res );
    PC += r.bytes;
    return r.cycles + page_crossed;
}

// TODO: account for page cross extra cycle in all instructions
int Cpu::exec() {
    uint8_t    opcode = load8( PC );
    InstRecord r      = Instruction::TABLE[opcode];
    switch ( opcode & 0b11 ) {
        case 0b00:
            return exec_00( r );
        case 0b01:
            return exec_01( r );
        case 0b10:
            return exec_10( r );
        case 0b11:
            throw IllegalOpcodeError("aaabbb00");
    }
    return 0;  // silence a compiler warning
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
    uint8_t   old_page;
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
            addr      = load16( PC + 1, kNoPageWrap );
            old_page  = addr.page;
            addr     += X;
            page_crossed =
                old_page == addr.page ? cPageNotCrossed : cPageCrossed;
            break;
        case mAbsoluteY:
            addr      = load16( PC + 1, kNoPageWrap );
            old_page  = addr.page;
            addr     += Y;
            page_crossed =
                old_page == addr.page ? cPageNotCrossed : cPageCrossed;
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
            addr      = load8( PC + 1 );
            addr      = load16( addr, kDoPageWrap );
            old_page  = addr.page;
            addr     += Y;
            page_crossed =
                old_page == addr.page ? cPageNotCrossed : cPageCrossed;
            break;

        case mImmediate:
            addr = PC + 1;
            break;
        case mRelative:
            addr      = PC + 2;
            old_page  = addr.page;
            addr     += static_cast<int>( load8( PC + 1 ) );
            page_crossed =
                old_page == addr.page ? cPageNotCrossed : cPageCrossed;
            break;
        case mAccumulator:
        case mImplied:
            return tuple( 0, page_crossed );  // TODO
    }
    return tuple( addr, page_crossed );
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
    // flags at bit positions 4 & 5 aren't actually in the NES CPU's 6-bit P register.
    // there is no convention on how to print these flags.
    // nintendulator's logging seems to print them as '10', hence the below
    // statement. helps to keep debugging the log consistent.
    const uint8_t true_P = P & ~(fBFlag) | (fUnusedFlag2);
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
        "{:02X}{:02X}  {:02X} {:2} {:2}  {:32}A:{:02X} X:{:02X} Y:{:02X} "
        "P:{:02X} SP:{:02X}"
        " PPU:{:8}"  // PPU fields placeholder
        "CYC:{}",
        std::make_format_args( PC.page, PC.index, opcode, arg1str.c_str(),
                               arg2str.c_str(), cur_inst.to_string().c_str(), A,
                               X, Y, true_P, SP, "", cycles_elapsed ) );
    std::cout << msg << std::endl;
}
