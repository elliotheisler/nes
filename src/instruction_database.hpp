#pragma once
#include <cstdint>
#include <string>
#include <optional>
#include "json.hpp"
#define INST_JSON_PATH "mos6502/python_scraping/instructions.json"
using json = nlohmann::json;

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
const char* const ADDRMODE_2_STRING[] = {
    [mZeroPage] = "ZeroPage",
    [mZeroPageX] = "ZeroPage,X",
    [mZeroPageY] = "ZeroPage,Y",

    [mAbsolute] = "Absolute",
    [mAbsoluteX] = "Absolute,X",
    [mAbsoluteY] = "Absolute,Y",

    [mIndirect] = "Indirect",
    [mIndirectX] = "(Indirect,X)",
    [mIndirectY] = "(Indirect),Y",

    [mImmediate] = "Immediate",
    [mAccumulator] = "Accumulator",
    [mImplied] = "Implied",
    [mRelative] = "Relative",
};

std::optional<AddrMode> string2AddrMode(const std::string& name);
std::string AddrMode2string(AddrMode mode);

class InstRecord {
  public:
    std::string name;
    std::string description;
    uint8_t opcode;
    AddrMode adr_mode; 
    int bytes;
    int cycles;

    bool is_empty() const;
};
const std::array<InstRecord, 256>& read_inst_db(const char* path) ;
void from_json(const json& j, InstRecord& ir) ;
std::ostream& operator<<(std::ostream& os, const InstRecord& ir) ;
json read_json(const char* path);
