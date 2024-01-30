#pragma once
#include <cstdint>
#include <optional>
#include <string>

#include "json.hpp"
#define INST_JSON_PATH "src/instructions_1.json"
using json = nlohmann::json;

class InstRecord;  // forward decl for the TABLE array

namespace Instruction {
    enum AddrMode {
        mZeroPage,
        mZeroPageX,
        mZeroPageY,

        mAbsolute,
        mAbsoluteX,
        mAbsoluteY,

        mIndirect,
        mIndirectX,
        mIndirectY,

        mImmediate,
        mAccumulator,
        mImplied,
        mRelative,
    };

    extern const std::array<InstRecord, 256> TABLE;
};  // namespace Instruction

// this has to be in global scope for template get() to work from json.hpp
class InstRecord {
   public:
    std::string name;
    std::string description;
    uint8_t opcode;
    Instruction::AddrMode adr_mode;
    int bytes;
    int cycles;
    bool pagecross_cycle;

    bool is_empty() const;
};

std::ostream& operator<<( std::ostream& os, const InstRecord& ir );
