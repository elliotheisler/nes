#include "instruction_database.hpp"
#include "json.hpp"
#include <iostream>
#include <bitset>
#include <fstream>
#include <optional>
using json = nlohmann::json;

std::optional<AddrMode> string2AddrMode(const std::string& name) {
    for (int i = 0; const char* s : ADDRMODE_2_STRING) {
        if (name == s) {
            return static_cast<AddrMode>(i);
        }
        i++;
    }
    return std::nullopt;
}

std::string AddrMode2string(AddrMode mode) {
    return ADDRMODE_2_STRING[mode];
}

bool InstRecord::is_empty() const {
    return bytes == 0 && cycles == 0;
}

std::ostream& operator<<(std::ostream& os, const InstRecord& ir) {
    os << "{\n"
        << "{  name  : " << ir.name << std::endl
        << "   desc  : " << ir.description  << std::endl
        << "   opcode: " << std::bitset<8>(ir.opcode) << std::endl
        << "   mode  : " << AddrMode2string(ir.adr_mode) << std::endl
        << "   bytes : " << ir.bytes << std::endl
        << "   cycles: " << ir.cycles << std::endl
        << "}\n";
    return os;
}

void from_json(const json& j, InstRecord& ir) {
    ir.name =  j.at("name").template get<std::string>();
    ir.description =  j.at("description").template get<std::string>();
    ir.opcode = std::stoi(
            j.at("opcode").template get<std::string>().substr(1), nullptr, 16
            );
    ir.adr_mode = string2AddrMode(
            j.at("mode").template get<std::string>()
            ).value();
    ir.bytes = std::stoi(
            j.at("bytes").template get<std::string>(), nullptr
            );
    ir.cycles = std::stoi(
            j.at("cycles").template get<std::string>(), nullptr
            );
}
    
const std::array<InstRecord, 256>& read_inst_db(const char* path) {
    static std::array<InstRecord, 256> db;

    const json json_db = read_json(path);
    for ( const json& json_record : json_db ) {
        InstRecord tmp;
        tmp = json_record.template get<InstRecord>();
        db[tmp.opcode] = tmp;
    }
    return db;
}

json read_json(const char* path) {
    std::ifstream f{ path };
    json data = json::parse( f );
    f.close();
    return data;
}
