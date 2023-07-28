#include <cstdlib>
#include <vector>
#include <cstddef>
#include <string>
#include <fstream>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.hpp"

using namespace std;

vector<u8> read_file(string& path) {
    ifstream ifs{ path };
    vector<u8> bytes;
    u8 cur;

    while (ifs >> cur) {
        bytes.push_back((u8) cur);
    }
    return bytes;
}


typedef struct {
    u32 prg_num_16kb_blks;
    u64 chr_num_8kb_blks;
    u8 mapper_no;
} INES;

INES parse_bytes(char *bytes) {
    // always begins with these 4 bytes
    if (strncmp(bytes, "NES\x1A", 4)) {
        fail("does not begin with \"NES\\x1A\"\n");
    }

    bool NES2_format = ( ( bytes[7] & 0b00001100 ) == 0b00001000 );

}
