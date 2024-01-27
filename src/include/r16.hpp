#include <cstdint>
/*! class for 16-bit registers in the CPU. encapsulates the
 * 8-bit page wraparound when adding address offsets, as seen in
 * some of the addressing modes.
 */
class r16 {
   public:
    uint8_t page;
    uint8_t index;
    // default default constructor: value initialization i.e. "r16{}" will
    // zero-init undefined fields
    r16() = default;
    // main constructor from 2 u8's
    r16(uint8_t p_page, uint8_t p_index);
    // implicit conversions to and from uint16_t
    r16(uint16_t u16);
    operator uint16_t() const;
    // adding 8-bit offsets like X, Y, etc, as well as ints ex: 'PC + 1'
    r16 operator+(uint8_t offset);
    r16 operator+(int offset);
    r16& operator+=(uint8_t offset);
    r16& operator++(int);
    // implementation common to above operators
   private:
    r16 add(uint8_t offset);

    // '*' means needs to be implemented
    // default uninitialized constructor
    // r16 = (*u16|(default)r16|(converted to u16)u8)
    // cast to uint16_t
    // r16 + (*u8|(same as adding unsigned?)i8|(converted to u8)int)
    /*
* types of atomic address accesses that occur
* as part of the various addressing modes
u8  -> u8  - (8bit address with 8bit result ex: zero page)
u16 -> u8  - etc...
u8  -> u16 w ...
u16 -> u16 w (16bit address with 16bit result, with pagewrap)
u16 -> u16 - (16bit address with 16bit result, no pagewrap)
*/
};