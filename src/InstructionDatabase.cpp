#include "InstructionDatabase.hpp"

#include <array>
#include <bitset>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>

#include "json.hpp"
using json = nlohmann::json;
using namespace Instruction;
using AddrMode = Instruction::AddrMode;
using enum Instruction::AddrMode;

std::optional<Instruction::AddrMode> string2AddrMode( const std::string& name );
std::string AddrMode2string( Instruction::AddrMode mode );

const std::array<InstRecord, 256>& read_inst_db( const char* path );
json read_json( const char* path );
void from_json( const json& j, InstRecord& ir );

const std::array<InstRecord, 256> Instruction::TABLE{
    read_inst_db( INST_JSON_PATH ) };

const char* const ADDRMODE_2_STRING[] = {
    [mZeroPage] = "ZeroPage",      [mZeroPageX] = "ZeroPage,X",
    [mZeroPageY] = "ZeroPage,Y",

    [mAbsolute] = "Absolute",      [mAbsoluteX] = "Absolute,X",
    [mAbsoluteY] = "Absolute,Y",

    [mIndirect] = "Indirect",      [mIndirectX] = "(Indirect,X)",
    [mIndirectY] = "(Indirect),Y",

    [mImmediate] = "Immediate",    [mAccumulator] = "Accumulator",
    [mImplied] = "Implied",        [mRelative] = "Relative",
};

std::optional<AddrMode> string2AddrMode( const std::string& name ) {
    for ( int i = 0; const char* s : ADDRMODE_2_STRING ) {
        if ( name == s ) {
            return static_cast<AddrMode>( i );
        }
        i++;
    }
    return std::nullopt;
}

std::string AddrMode2string( AddrMode mode ) { return ADDRMODE_2_STRING[mode]; }

bool InstRecord::is_empty() const { return bytes == 0 && cycles == 0; }

std::ostream& operator<<( std::ostream& os, const InstRecord& ir ) {
    os << "{\n"
       << "{  name  : " << ir.name << std::endl
       << "   desc  : " << ir.description << std::endl
       << "   opcode: " << std::bitset<8>( ir.opcode ) << std::endl
       << "   mode  : " << AddrMode2string( ir.adr_mode ) << std::endl
       << "   bytes : " << ir.bytes << std::endl
       << "   cycles: " << ir.cycles << std::endl
       << "}\n";
    return os;
}

void from_json( const json& j, InstRecord& ir ) {
    ir.name        = j.at( "name" ).template get<std::string>();
    ir.description = j.at( "description" ).template get<std::string>();
    ir.opcode      = j.at( "opcode" ).template get<uint8_t>();
    ir.adr_mode =
        string2AddrMode( j.at( "mode" ).template get<std::string>() ).value();
    ir.bytes  = j.at( "bytes" ).template get<int>();
    ir.cycles = j.at( "cycles" ).at( "count" ).template get<int>();
    ir.pagecross_cycle =
        j.at( "cycles" ).at( "add_on_pagecross" ).template get<bool>();
}

const std::array<InstRecord, 256>& read_inst_db( const char* path ) {
    static std::array<InstRecord, 256> db;

    const json json_db = read_json( path );
    for ( const json& json_record : json_db ) {
        InstRecord tmp;
        tmp            = json_record.template get<InstRecord>();
        db[tmp.opcode] = tmp;
    }
    return db;
}

json read_json( const char* path ) {
    std::ifstream f{ path };
    json data = json::parse( f );
    f.close();
    return data;
}
