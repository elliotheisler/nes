#pragma once
#include <cstdint>
#include <string>
#include "json.hpp"
#define INST_JSON_PATH "mos6502/python_scraping/instructions.json"
using json = nlohmann::json;

class InstRecord {
  public:
    std::string name;
    std::string description;
    uint8_t opcode;
    std::string adr_mode; // TODO: make this an enum
    int bytes;
    int cycles;

    bool is_empty() const;
};
const std::array<InstRecord, 256>& read_inst_db(const char* path) ;
void from_json(const json& j, InstRecord& ir) ;
std::ostream& operator<<(std::ostream& os, const InstRecord& ir) ;
json read_json(const char* path);
