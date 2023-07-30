#include "instruction_database.hpp"
#include <iostream>

int main() {
    std::array<InstRecord, 256> inst_db = read_inst_db(INST_JSON_PATH);
    for (const InstRecord& ir : inst_db) {
        if (!ir.is_empty())
            std::cout << ir;
    }
}
