#include "Cpu.hpp"

int main() {
    Cpu cpu{0, 0, 0, 0, 0, 0xC000};
    for (int i = 0; i < 1e2; i++) {
        cpu.clock();
    }
}
