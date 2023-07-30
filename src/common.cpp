#include <cstdint>
#include <cmath>
#include <cassert>
#include <cstring>
#include <iostream>
#include "json.hpp"

uint16_t select_bitrange(uint16_t word, int lower, int upper) {
    const uint16_t mask = pow(2, 1 + upper - lower) - 1;
    std::cout << "mask - " << lower << ", " << upper << ": " << mask << '\n';
    return (word & mask) >> lower;
}

uint16_t select_bitrange(uint16_t word, uint16_t mask) {
    uint16_t ret = (word & mask) ;
    uint16_t tmp;
    // shift right until we hit a '1' bit
    while ((tmp = (ret >> 1) << 1) == ret) {
            ret = tmp >> 1;
    }
    return ret;
}

// meant to mimic verilog's "casez" bit matching syntax
bool check_bits(const char* bitmask_str, uint16_t word) {
    int i;
    const char* c;
    for (i = 15, c = bitmask_str; i >= 0 && *c != '\0'; i--, c++) {
        switch (*c) {
            case '1':
                if (!(word & (1 << i)))
                    return false;
                break;
            case '0':
                if (word & (1 << i))
                    return false;
                break;
            case '_':
                i++; // don't decrement bit position
                break;
            case 'x':
            case 'z':
                continue;
            default:
                throw "incorrectly formatted bitmask_str";
        }
    }
    if (*c == '\0' && i >  -1) 
        throw "bitmask_str too short < 16 bits";
    if (*c != '\0' && i <= -1) 
        throw "bitmask_str too long  > 16 bits";
    return true;
}

