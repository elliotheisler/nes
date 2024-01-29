#include <cstdint>

#include "InstructionDatabase.hpp"

namespace Disassemble {
    class ParsedInst {
       public:
        const uint16_t rom_addr;
        const uint8_t arg1, arg2;
        const InstRecord& record;
        // TODO: use smart pointers
        ParsedInst( uint16_t p_rom_addr, uint8_t opcode, uint8_t p_arg1,
                    uint8_t p_arg2 );
        ParsedInst( uint8_t* real_addr, uint16_t p_rom_addr );

        std::string to_string() const;

       private:
    };
};  // namespace Disassemble

// std::ostream& operator<<( std::ostream& os, const Disassemble::ParsedInst& ir
// );
