#include "CpuAndPpu.hpp"
#include <cstdint>

r16::operator int() const {
    return static_cast<int>(page << 8) | index;
}

r16 r16::operator+(uint8_t other) {
    r16 next {*this};
    next.index += other;
    if (next.index < this->index)
        next.page++;
    return next;
}

r16 r16::operator+(int other) {
    r16 next {*this};
    next.index += other;
    if (next.index < this->index)
        next.page++;
    return next;
}

r16 r16::operator++(int) {
    return operator+(1);
}

r16 r16::operator=(uint8_t val) {
    page = 0;
    index = val;
    return *this;
}

r16 r16::operator+=(uint8_t val) {
    r16 ret;
    ret.page = page;
    ret.index = index + val;
    return ret;
}
