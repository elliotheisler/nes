#include "Cpu.hpp"

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "Cartridge.hpp"
#include "Mapper.hpp"
#include "instruction_database.hpp"

using json = nlohmann::json;

Cpu::Cpu(Cartridge& p_cartridge)
    : cartridge{p_cartridge}, PC{0}, A{0}, SP{0}, X{0}, Y{0}, SR{0} {
    do_poweron();
};

void Cpu::clock() {
    counter++;
    cycles_elapsed++;
    if (get_cycles(load8(PC)) == counter) {
        exec();
        log_nintendulator();
        counter = 0;
    }
}

void Cpu::exec() {
    // https://llx.com/Neil/a2/opcodes.html
    const uint8_t opcode = load8(PC);
    const int cycles     = get_cycles(opcode);
    const int num_bytes  = get_num_bytes(opcode);
    const AddrMode mode  = get_mode(opcode);

    // load and store
    uint8_t* reg_arg;
    std::optional<Cpu::r16> effective_addr = get_effective_addr(mode);
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
    // stores: have 8th bit set
    if ((0b11100000 & opcode) >> 5 == 0b100) {
        store8(effective_addr.value(), *reg_arg);
    }
    // loads: have 8th and 6th bits set
    else if ((0b11100000 & opcode) >> 5 == 0b101) {
        *reg_arg = load8(effective_addr.value());
    }

    PC = PC + num_bytes;
}

AddrMode Cpu::get_mode(uint8_t opcode) { return inst_db[opcode].adr_mode; }

int Cpu::get_cycles(uint8_t opcode) { return inst_db[opcode].cycles; }

int Cpu::get_num_bytes(uint8_t opcode) { return inst_db[opcode].bytes; }

uint8_t Cpu::get_affected_flags(uint8_t opcode) {  // TODO
    return 0;
}

std::string Cpu::to_assembly(const uint8_t opcode) {
    const InstRecord i_record = inst_db[opcode];
    return i_record.name;
}

std::optional<Cpu::r16> Cpu::get_effective_addr(AddrMode m) {
    using enum PageWrap;
    Cpu::r16 addr{0x0000};
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
            addr.index += load8(PC + 1);
            break;
        case mAccumulator:
        case mImplied:
            return std::nullopt;
    }
    return addr;
}

void Cpu::do_poweron() {
    // https://www.nesdev.org/wiki/CPU_power_up_state#At_power-up
    cycles_elapsed = 7; /* nestest.log starts with 7 cycles elapsed, so I assume
                           the poweron sequence is like BRK and INT */
    SR = 0b00110100;
    A = X = Y = 0x00;
    SP        = 0b11111101;
    store8(0x4017, 0);  // frame irq enabled
    store8(0x4015, 0);  // all channels disabled
    for (uint16_t addr = 0x4000; addr <= 0x4005; addr++) {
        store8(addr, 0);
    }
    for (uint16_t addr = 0x4010; addr <= 0x4013; addr++) {
        store8(addr, 0);
    }
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
    log_nintendulator();
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

Cpu::r16 Cpu::load16(r16 addr, PageWrap pw) {
    using enum Cpu::PageWrap;
    Cpu::r16 ret{0x0000};
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
// TODO: could add load/stores for zero page i.e. load8(addr.index);
uint8_t Cpu::load8(uint16_t addr) { return addr_access<kLoad>(addr, 0); }

void Cpu::store8(uint16_t addr, uint8_t payload) {
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
                // FINALLY: cartridge space
                return cartridge.cpu_access<A>(addr, payload);
            } else if ((addr & 0b11000) == 0b11000) {
                // SECOND; 8 bytes: "APU and I/O functionality that is normally
                // disabled." See CPU Test Mode
            } else {
                // FIRST 3 * 8 bytes: NES APU and I/O registers
            }
    }
    return 0;
}

// =================logging

void Cpu::log_nintendulator() {
    fprintf(stdout, "%02X%02X %s A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d\n",
            PC.page, PC.index, inst_db[load8(PC)].name.c_str(), A, X, Y, SR, SP,
            cycles_elapsed);
}

// =====================json database stuff

const std::array<InstRecord, 256> Cpu::inst_db{read_inst_db(INST_JSON_PATH)};

// ===================r16 type

Cpu::r16::r16(uint16_t i) {
    page  = i >> 8;
    index = i & 0b11111111;
}

Cpu::r16 Cpu::r16::operator+(uint8_t other) {
    r16 next{*this};
    next.index += other;
    if (next.index < this->index) next.page++;
    return next;
}

Cpu::r16 Cpu::r16::operator+(int other) {
    r16 next{*this};
    next.index += other;
    if (next.index < this->index) next.page++;
    return next;
}

Cpu::r16 Cpu::r16::operator++(int) { return operator+(1); }

Cpu::r16 Cpu::r16::operator=(uint8_t val) {
    page  = 0;
    index = val;
    return *this;
}

Cpu::r16 Cpu::r16::operator+=(uint8_t val) {
    r16 ret{0};
    ret.page  = page;
    ret.index = index + val;
    return ret;
}

Cpu::r16::operator uint16_t() const {
    return (static_cast<uint16_t>(page) << 8) | index;
}