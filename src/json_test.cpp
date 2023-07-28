#include "json.hpp"
#include <fstream>
#include <iostream>
using json = nlohmann::json;
int main() {
    std::ifstream f{ "instructions.json" };
    json data = json::parse(f);
    std::cout << data;
    f.close();
}
