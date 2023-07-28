#include "common.hpp"
#include "cassert"
#include <iostream>

int main() {
    std::cout << "testing...\n";
    // simple cases
    assert(check_bits("0000000000000001", 0b01) == true);
    assert(check_bits("0000000000000001", 0b11) == false);
    assert(check_bits("0000000000000010", 0b11) == false);
    assert(check_bits("0000000000000010", 0b10) == true);
    // simple cases with 'x' and 'z'
    assert(check_bits("00000000000000x1", 0b11) == true);
    assert(check_bits("00000000000001x1", 0b001) == false);
    assert(check_bits("00000000000001z1", 0b001) == false);
    assert(check_bits("x00x0z00z00001z1", 0b001) == false);
    // a "catch-all" case -- notice all 1's and 0's line up, so this should be
    // true
    assert(check_bits("x10x0z00z00001z1", 
                     0b0101010010000101 ) == true);
    // now check everything with random underscores
    assert(check_bits("0000000000__00xx01", 0b1) == true);
    assert(check_bits("0000000000_00x_x10", 0b10) == true);

    assert(check_bits("_0000000000__0011x1", 0b1101) == true);
    assert(check_bits("_0000000000__0011z1", 0b1111) == true);
    assert(check_bits("_0000000000__0011x1", 0b1110) == false);

    assert(check_bits("00_0_0_000000__00011x", 0b110) == true);
    assert(check_bits("00_0_0_000000__00011z", 0b1110) == false);


    // exceptions cases
    
    // bitmask contains wrong characters
    bool except_raised = false;
    try {
        check_bits("00_0_0_000h00__00011z", 0b1110);
    } catch (...) {
        except_raised = true;
    }
    assert(except_raised == true);

    // bitmask contains too many chars
    except_raised = false;
    try {
        check_bits("00_0_0_00000__00011z0000000000", 0b1110);
    } catch (...) {
        except_raised = true;
    }
    assert(except_raised == true);

    // bitmask contains not enough chars
    except_raised = false;
    try {
        check_bits("00_0_0_00000", 0b1110);
    } catch (...) {
        except_raised = true;
    }
    assert(except_raised == true);
    std::cout << "testing complete.\n";
}
