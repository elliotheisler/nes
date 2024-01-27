#include "r16.hpp"

#include <cstdint>
r16::r16(uint8_t p_page, uint8_t p_index) {
    page  = p_page;
    index = p_index;
}
r16::r16(uint16_t u16) {
    page  = u16 >> 8;
    index = u16;  // TEST: is this compile-time or runtime casted to uint8_t?
}

r16::operator uint16_t() const {
    // TEST: will index be promoted to u16?
    return (static_cast<uint16_t>(page) << 8) | index;
}
r16 r16::operator+(uint8_t offset) { return this->add(offset); }
r16 r16::operator+(int offset) { return this->add(offset); }
r16& r16::operator+=(uint8_t offset) {
    *this = this->add(offset);
    return *this;
}
r16& r16::operator++(int) {
    *this = this->add(1);
    return *this;
}
r16 r16::add(uint8_t offset) {
    r16 res;
    res.index = index + offset;
    res.page  = res.index > index ? page : page + 1;
    return res;
}