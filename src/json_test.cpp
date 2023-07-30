#include "instruction_database.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>
using json = nlohmann::json;
int main() {
    std::ifstream f{ INST_JSON_PATH };
    json data = json::parse(f);
    std::cout << data;
    f.close();
}
