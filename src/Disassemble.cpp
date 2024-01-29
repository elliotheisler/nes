#include "Disassemble.hpp"

#include <format>

#include "InstructionDatabase.hpp"
using namespace Disassemble;

ParsedInst::ParsedInst( uint16_t p_rom_addr, uint8_t opcode, uint8_t p_arg1,
                        uint8_t p_arg2 )
    : rom_addr{ p_rom_addr },
      arg1{ p_arg1 },
      arg2{ p_arg2 },
      record{ Instruction::TABLE[opcode] } {};


ParsedInst::ParsedInst( uint8_t* real_addr, uint16_t p_rom_addr )
    : rom_addr{ p_rom_addr },
      record{ ::Instruction::TABLE[*real_addr] },
      arg1{ ::Instruction::TABLE[*real_addr].bytes >= 2
                ? *( real_addr + 1 )
                : static_cast<uint8_t>( 0 ) },
      arg2{ ::Instruction::TABLE[*real_addr].bytes >= 3
                ? *( real_addr + 1 )
                : static_cast<uint8_t>( 0 ) } {};

std::string ParsedInst::to_string() const {
    // TODO
    using namespace Instruction;
    using enum Instruction::AddrMode;
    constexpr static const char* ADDR_MODE_FMTS[] = {
        [mZeroPage]  = "${0:02X}",
        [mZeroPageX] = "${0:02X},X",
        [mZeroPageY] = "${0:02X},Y",

        [mAbsolute]  = "${1:02X}{0:02X}",
        [mAbsoluteX] = "${1:02X}{0:02X},X",
        [mAbsoluteY] = "${1:02X}{0:02X},Y",

        [mIndirect]  = "(${1:02X}{0:02X})",
        [mIndirectX] = "(${0:02X},X)",
        [mIndirectY] = "(${0:02X}),Y",

        [mImmediate]   = "#${0:02X}",
        [mAccumulator] = "",
        [mImplied]     = "",
        [mRelative]    = "${0:02X}",
    };
    const std::string fmt = ADDR_MODE_FMTS[record.adr_mode];

    std::string address_part =
        std::vformat( fmt, std::make_format_args( arg1, arg2 ) );
    return std::vformat( "{} {}",
                         std::make_format_args( record.name, address_part ) );
}

// std::ostream& operator<<( std::ostream& os, const ParsedInst& ir ) {
//     using enum Instruction::AddrMode;
//     unsigned int int_arg1 = static_cast<unsigned int>( ir.arg1 );
//     unsigned int int_arg2 = static_cast<unsigned int>( ir.arg2 );
//     std::stringstream ss{};
//     ss << ir.record.name << " ";
//     ss << std::uppercase << std::setfill( '0' ) << std::setw( 2 ) <<
//     std::hex; switch ( ir.record.adr_mode ) {
//         case mZeroPage:
//             ss << '$';
//             ss << int_arg1;
//             break;
//         case mZeroPageX:
//             ss << '$';
//             ss << int_arg1;
//             ss << ",X";
//             break;
//         case mZeroPageY:
//             ss << '$';
//             ss << int_arg1;
//             ss << ",Y";
//             break;
//
//         case mAbsolute:
//             ss << "$";
//             ss << int_arg2 << int_arg1;
//             break;
//         case mAbsoluteX:
//             ss << '$';
//             ss << int_arg2 << int_arg1;
//             ss << ",X";
//             break;
//         case mAbsoluteY:
//             ss << '$';
//             ss << int_arg2 << int_arg1;
//             ss << ",Y";
//             break;
//
//         case mIndirect:
//             ss << "($";
//             ss << int_arg2 << int_arg1;
//             ss << ")";
//             break;
//         case mIndirectX:
//             ss << "($";
//             ss << int_arg1;
//             ss << ",X)";
//             break;
//         case mIndirectY:
//             ss << "($";
//             ss << int_arg1;
//             ss << "),Y";
//             break;
//         case mImmediate:
//             ss << "#$";
//             ss << int_arg2 << int_arg1;
//             break;
//         case mRelative:
//             ss << '$';
//             ss << int_arg1;
//             break;
//         case mAccumulator:
//         case mImplied:
//             break;
//     }
//     return os << ss.str();
// }
