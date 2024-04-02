# NES
This repository contains my incomplete attempt at a NES emulator, which in its current state is just a 6502 emulator. The cpu test driver is able to match Nintendulator's output of the golden log of nestest until unofficial opcodes show up. It can be built with `scons bin/test_Cpu`.
In attempting this project I learned some C++ and digital logic.
### TODO
- make the CPU cycle-accurate instead of stepping per-instruction
- implement the PPU
- handle controller input
- handle audio output
