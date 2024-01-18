#include "Cpu.hpp"

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

#include "Mapper.hpp"
#include "instruction_database.hpp"
#include "register.hpp"
using json       = nlohmann::json;
using AccessType = XpuBase::AccessType;
using enum XpuBase::AccessType;

Cpu::Cpu() {}

Cpu::Cpu(uint8_t m_A, uint8_t m_Y, uint8_t m_X, uint8_t m_SP, uint8_t m_SR,
         uint16_t m_PC)
    : A{m_A}, Y{m_Y}, X{m_X}, SP{m_SP}, SR{m_SR}, PC{m_PC} {}

void Cpu::clock() {
    counter++;
    cycles_elapsed++;
    if (get_cycles(load8(PC)) == counter) {
        exec();
        counter = 0;
    }
    log_nintendulator();
}

void Cpu::exec() {
    std::cout << "exec\n";
    // https://llx.com/Neil/a2/opcodes.html
    const uint8_t opcode = load8(PC);
    const int cycles     = get_cycles(opcode);
    const int num_bytes  = get_num_bytes(opcode);
    const AddrMode mode  = get_mode(opcode);

    // load and store
    uint8_t* reg_arg;
    r16 effective_addr = get_effective_addr(mode);
    switch (opcode & 0b00000011) {
        case 0b00:
            reg_arg = &Y;
            break;
        case 0b01:
            reg_arg = &X;
            break;
        case 0b10:
            reg_arg = &A;
            break;
        default:
            throw std::logic_error("unofficial opcode");
    }
    // store
    if (check_bits("100_xxx_xx", opcode)) {
        store8(effective_addr, *reg_arg);
    }
    // load
    else if (check_bits("101_xxx_xx", opcode)) {
        *reg_arg = load8(effective_addr);
    }

    PC = PC + num_bytes;
    log_nintendulator();
}

AddrMode Cpu::get_mode(uint8_t opcode) { return inst_db[opcode].adr_mode; }

int Cpu::get_cycles(uint8_t opcode) { return inst_db[opcode].cycles; }

int Cpu::get_num_bytes(uint8_t opcode) { return inst_db[opcode].bytes; }

uint8_t Cpu::get_affected_flags(uint8_t opcode) {  // TODO
    return 0;
}

r16 Cpu::get_effective_addr(AddrMode m) {
    using enum PageWrap;
    r16 addr{0x0000};
    switch (m) {
        case mZeroPage:
            addr = load8(PC + 1);
            break;
        case mZeroPageX:
            addr        = load8(PC + 1);
            addr.index += X;
            break;
        case mZeroPageY:
            addr        = load8(PC + 1);
            addr.index += Y;
            break;

        case mAbsolute:
            addr = load16(PC + 1, kNoPageWrap);
            break;
        case mAbsoluteX:
            addr  = load16(PC + 1, kNoPageWrap);
            addr += X;
            break;
        case mAbsoluteY:
            addr  = load16(PC + 1, kNoPageWrap);
            addr += Y;
            break;

        case mIndirect:
            addr.index = load8(PC + 1);
            addr       = load16(addr, kDoPageWrap);
        case mIndirectX:
            addr        = load8(PC + 1);
            addr.index += X;
            addr        = load16(addr, kDoPageWrap);
            break;
        case mIndirectY:
            addr.index  = load8(PC + 1);
            addr        = load16(addr, kDoPageWrap);
            addr       += Y;
            break;

        case mImmediate:
            addr = PC + 1;
            break;
        case mRelative:
            addr        = PC;
            addr.index += static_cast<int8_t>(load8(PC + 1));
            break;
        case mAccumulator:
            throw std::logic_error(
                "accumulator mode does not have an effective address");
        case mImplied:
            throw std::logic_error(
                "implied mode does not have an effective address");
    }
    return addr;
}

void Cpu::do_poweron() {
    // https://www.nesdev.org/wiki/CPU_power_up_state#At_power-up
    //     SR = 0b00110100;
    //     A = X = Y = 0x00;
    //     SP        = 0b11111101;
    //     store8(0x4017, 0);  // frame irq enabled
    //     store8(0x4015, 0);  // all channels disabled
    //     for (uint16_t addr = 0x4000; addr <= 0x4005; addr++) {
    //         store8(addr, 0);
    //     }
    //     for (uint16_t addr = 0x4010; addr <= 0x4013; addr++) {
    //         store8(addr, 0);
    //     }
    // TODO:
    //     All 15 bits of noise channel LFSR = $0000[5]. The first time the LFSR
    //     is clocked from the all-0s state, it will shift in a 1. APU Frame
    //     Counter:
    //
    //         2A03E, G, various clones: APU Frame Counter reset.
    //         2A03letterless: APU frame counter powers up at a value equivalent
    //         to 15
    //
    //     Internal memory ($0000-$07FF) has unreliable startup state. Some
    //     machines may have consistent RAM contents at power-on, but others do
    //     not.
    //
    //         Emulators often implement a consistent RAM startup state (e.g.
    //         all $00 or $FF, or a particular pattern), and flash carts like
    //         the PowerPak may partially or fully initialize RAM before
    //         starting a program, so an NES programmer must be careful not to
    //         rely on the startup contents of RAM.
}

void Cpu::do_reset() {
    // https://www.nesdev.org/wiki/CPU_power_up_state#After_reset
    //     using enum Cpu::CpuFlag;
    //     SP -= 3;
    //     set_flag(fInterruptDisable, true);
    //     store8(0x4015, 0);
    // TODO:
    //     APU triangle phase is reset to 0 (i.e. outputs a value of 15, the
    //     first step of its waveform) APU DPCM output ANDed with 1 (upper 6
    //     bits cleared)
    //
    //         APU Frame Counter:
    //         2A03E, G, various clones: APU Frame Counter reset.
    //         2A03letterless: APU frame counter retains old value [6]
}

r16 Cpu::load16(r16 addr, PageWrap pw) {
    using enum Cpu::PageWrap;
    r16 ret{0x0000};
    ret.index = load8(addr);
    switch (pw) {
        case kDoPageWrap:
            addr.index++;
            break;
        case kNoPageWrap:
            addr++;
    }
    ret.page = load8(addr);
    return ret;
}

uint8_t Cpu::load8(r16 addr) { return addr_access<kLoad>(addr, 0); }

uint8_t Cpu::load8(uint16_t addr) { return addr_access<kLoad>(addr, 0); }

void Cpu::store8(r16 addr, uint8_t payload) {
    addr_access<kStore>(addr, payload);
}
template <AccessType A>
uint8_t Cpu::addr_access(uint16_t addr, uint8_t payload) {
    // https://www.nesdev.org/wiki/CPU_memory_map

    switch (addr >> 13) {
        case 0b000:  // 2 KB internal RAM
            switch (A) {
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
            if (addr > 0b0100000000011111) {
                // After the first 4 * 8 bytes: cartridge space
            } else if ((addr & 0b11000) == 0b11000) {
                // 8 bytes: APU and I/O functionality that is normally disabled.
                // See CPU Test Mode.
            } else {
                // First 3 * 8 bytes: NES APU and I/O registers
            }
    }
    return 0;
}

void Cpu::log_nintendulator() {
    fprintf(stdout, "%04X A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d\n",
            static_cast<unsigned int>(PC), A, X, Y, SR, SP, cycles_elapsed);
    //     std::cout << std::vformat(
    //         "{} A:{}, X:{}, Y:{}, P:{}, SP:{}, CYC:{}\n",
    //         std::make_format_args(PC, A, X, Y, SR, SP, cycles_elapsed));
}

// json database stuff

const std::array<InstRecord, 256> Cpu::inst_db{read_inst_db(INST_JSON_PATH)};
