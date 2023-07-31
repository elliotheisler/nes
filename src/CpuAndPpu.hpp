#pragma once
#include <cstdint>
class MemAccess {
  public:
    enum class AccessType {
        kLoad,
        kStore
    };
    AccessType type;
    uint16_t addr;
    uint8_t payload;

    template <AccessType T>
    MemAccess(uint16_t new_addr): addr{ new_addr } {
        static_assert(T == AccessType::kLoad,"tried to create a store MemAccess without providing a payload argument");
    }

    template <AccessType T>
    MemAccess(uint16_t new_addr, uint8_t new_payload): addr{ new_addr }, payload {new_payload} {
        static_assert(T == AccessType::kStore,"tried to create a load MemAccess with redundant payload argument provided");
    }

};
