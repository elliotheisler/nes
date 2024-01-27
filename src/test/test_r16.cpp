#include <cassert>
#include <cstdint>

#include "r16.hpp"

int main() {
    // TEST: construct by two uints
    r16 r0{0xAB, 0xCD};
    assert(r0.index == 0xCD && r0.page == 0xAB);
    // TEST: implicit default copy constructor
    r16 r1 = r0;
    assert(r1 == r0);
    // checking if these are seperate copies...
    r1.index = 0;
    assert(r1 != r0);
    // TEST: uint16_t conversion constructor
    uint16_t u16 = 0xABCD;
    r16 r2       = u16;
    assert(r2.index == 0xCD && r2.page == 0xAB);
    // TEST: initialization from single uint8_t by implicit conversion to
    // uint16_t
    uint8_t ex = 0xFF;
    r2         = ex;
    assert(r2.index == 0xFF && r2.page == 0x00);
    // TEST: conversion to uint16_t
    u16 = r16{0x12, 0x34};
    assert(u16 == 0x1234);
    // TEST: addition operations
    assert(r16(0xFFFF) + static_cast<uint8_t>(1) == 0);
    assert(r16(0xFFFF) + 1 == 0);
    r16 r3{0x1234};
    assert((r3 += 1) == 0x1235);
    assert(r3 == 0x1235);
    r3++;
    assert(r3 == 0x1236);
}