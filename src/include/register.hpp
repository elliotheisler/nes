#pragma once
#include <cstdint>

/*! class for 16-bit registers in the CPU. encapsulates the
 * 8-bit page wraparound when adding address offsets, as seen in
 * some of the addressing modes.
 */
class r16 {
   public:
    uint8_t page;
    uint8_t index;
    r16(uint16_t i);
    operator unsigned int() const;

    r16 operator+(uint8_t other);
    r16 operator+(int other);

    r16 operator++(int);

    r16 operator=(uint8_t val);
    r16 operator+=(uint8_t val);
};

/*! any functionality common to both CPU and PPU. see below
 * documentation
 */
class ProcessorBase {
   public:
    // define the address space with respect to loads/stores
    // using this function
    // TODO: use CRTP for static polymorphism here
   public:
    enum class AccessType { kLoad, kStore };

    template <AccessType A>
    uint8_t addr_access(r16 addr, uint8_t payload);
};

/*
 * types of atomic address accesses that occur
 * as part of the various addressing modes
 u8  -> u8  - (8bit address with 8bit result ex: zero page)
 u16 -> u8  - etc...
 u8  -> u16 w ...
 u16 -> u16 w (16bit address with 16bit result, with pagewrap)
 u16 -> u16 - (16bit address with 16bit result, no pagewrap)
*/
