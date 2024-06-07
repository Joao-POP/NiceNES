#include "Cartridge.h"
#include "Mapper.h"

namespace sn {
Cartridge::Cartridge()
    : m_nameTableMirroring(0), m_mapperNumber(0), m_extendedRAM(false) {}
const std::vector<Byte> &Cartridge::getROM() { return m_PRG_ROM; }

const std::vector<Byte> &Cartridge::getVROM() { return m_CHR_ROM; }

Byte Cartridge::getMapper() { return m_mapperNumber; }

Byte Cartridge::getNameTableMirroring() { return m_nameTableMirroring; }

bool Cartridge::hasExtendedRAM() {
  // Some ROMs don't have this set correctly, plus there's no particular reason
  // to disable it.
  return true;
}

bool Cartridge::loadFromFile(std::string path) {
  std::ifstream romFile(path, std::ios_base::binary | std::ios_base::in);
  if (!romFile) {
    std::clog << "Could not open ROM file from path: " << path << std::endl;
    return false;
  }

  std::vector<Byte> header;
  std::clog << "Reading ROM from path: " << path << std::endl;

  // Header
  header.resize(0x10);
  if (!romFile.read(reinterpret_cast<char *>(&header[0]), 0x10)) {
    std::clog << "Reading iNES header failed." << std::endl;
    return false;
  }
  if (std::string{&header[0], &header[4]} != "NES\x1A") {
    std::clog << "Not a valid iNES image. Magic number: " << std::hex
              << header[0] << " " << header[1] << " " << header[2] << " "
              << int(header[3]) << std::endl
              << "Valid magic number : N E S 1a" << std::endl;
    return false;
  }

  std::clog << "Reading header, it dictates: \n";

  Byte banks = header[4];
  std::clog << "16KB PRG-ROM Banks: " << +banks << std::endl;
  if (!banks) {
    std::clog << "ROM has no PRG-ROM banks. Loading ROM failed." << std::endl;
    return false;
  }

  Byte vbanks = header[5];
  std::clog << "8KB CHR-ROM Banks: " << +vbanks << std::endl;

  if (header[6] & 0x8) {
    m_nameTableMirroring = NameTableMirroring::FourScreen;
    std::clog << "Name Table Mirroring: "
              << "FourScreen" << std::endl;
  } else {
    m_nameTableMirroring = header[6] & 0x1;
    std::clog << "Name Table Mirroring: "
              << (m_nameTableMirroring == 0 ? "Horizontal" : "Vertical")
              << std::endl;
  }

  m_mapperNumber = ((header[6] >> 4) & 0xf) | (header[7] & 0xf0);
  std::clog << "Mapper #: " << +m_mapperNumber << std::endl;

  m_extendedRAM = header[6] & 0x2;
  std::clog << "Extended (CPU) RAM: " << std::boolalpha << m_extendedRAM
            << std::endl;

  if (header[6] & 0x4) {
    std::clog << "Trainer is not supported." << std::endl;
    return false;
  }

  if ((header[0xA] & 0x3) == 0x2 || (header[0xA] & 0x1)) {
    std::clog << "PAL ROM not supported." << std::endl;
    return false;
  } else
    std::clog << "ROM is NTSC compatible.\n";

  // PRG-ROM 16KB banks
  m_PRG_ROM.resize(0x4000 * banks);
  if (!romFile.read(reinterpret_cast<char *>(&m_PRG_ROM[0]), 0x4000 * banks)) {
    std::clog << "Reading PRG-ROM from image file failed." << std::endl;
    return false;
  }

  // CHR-ROM 8KB banks
  if (vbanks) {
    m_CHR_ROM.resize(0x2000 * vbanks);
    if (!romFile.read(reinterpret_cast<char *>(&m_CHR_ROM[0]),
                      0x2000 * vbanks)) {
      std::clog << "Reading CHR-ROM from image file failed." << std::endl;
      return false;
    }
  } else
    std::clog << "Cartridge with CHR-RAM." << std::endl;
  return true;
}
} // namespace sn
