#pragma once
#include <cstdint>

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

/*
 * wrap/no wrap
 */

class XpuBase {
   public:
    enum class AccessType { kLoad, kStore };
    // define the address space with respect to loads/stores
    // using this function
    // TODO: use CRTP for static polymorphism here
   public:
    template <AccessType A>
    uint8_t addr_access(r16 addr, uint8_t payload);
};

/*
 u8  -> u8  -
 u16 -> u8  -
 u8  -> u16 w
 u16 -> u16 w
 u16 -> u16 -
*/
