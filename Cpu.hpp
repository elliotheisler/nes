#include <cstdint>
#include "Mapper.hpp"
class Cpu {
  public:
    Mapper mapper;
    uint8_t ram[1 << 11];

    Cpu(Mapper new_mapper);
    
    uint8_t* apply_addr(uint16_t addr);

     
};
