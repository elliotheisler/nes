#pragma once
#include <cstdint>
#include "json.hpp"
#define INST_JSON_PATH "lib/mos6502/python_scraping/instructions.json"

using json = nlohmann::json;

/* Integer type shortcuts */
typedef uint8_t  u8;  typedef int8_t  s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
typedef uint64_t u64; typedef int64_t s64;
uint16_t select_bitrange(uint16_t word, int lower, int upper);
uint16_t select_bitrange(uint16_t word, uint16_t mask);
bool check_bits(const char* bitmask_str, uint16_t word);
json read_json(const char* path);
