#include <cstdint>
#include <string>
#include <vector>
class Cartridge {
   public:
    void read_file(std::string&& path);

   private:
    std::vector<uint8_t> prg_rom;
    std::vector<uint8_t> chr_rom;
};
