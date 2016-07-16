#include "Emulator.h"

namespace sn
{
    Emulator::Emulator() :
        m_cpu(m_bus),
        m_ppu(m_pictureBus),
        m_cycleTimer(0)
    {
        m_bus.setReadCallback(PPUSTATUS, [&m_ppu](void) {return m_ppu.getStatus();});
        m_bus.setReadCallback(PPUDATA, [&m_ppu](void) {return m_ppu.getData();});
        m_bus.setReadCallback(OAMDATA, [&m_ppu](void) {return m_ppu.getOAMData();});

        m_bus.setWriteCallback(PPUCTRL, [&m_ppu](Byte b) {m_ppu.control(b);});
        m_bus.setWriteCallback(PPUMASK, [&m_ppu](Byte b) {m_ppu.setMask(b);});
        m_bus.setWriteCallback(OAMADDR, [&m_ppu](Byte b) {m_ppu.setOAMAddress(b);});
        m_bus.setWriteCallback(PPUSCROL, [&m_ppu](Byte b) {m_ppu.setScroll(b);});
        m_bus.setWriteCallback(PPUDATA, [&m_ppu](Byte b) {m_ppu.setData(b);});

    }

    void Emulator::run(std::string rom_path)
    {
        if (!m_cartridge.loadFromFile(rom_path))
            return;

        if (m_bus.loadCartridge(&m_cartridge) &&
            m_pictureBus.loadCartridge(&m_cartridge))
            return;

        //The Loop (R) (tm)
        sf::Event event;
        while (m_window.isOpen())
        {
            while (m_window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                    m_window.close();
            }

            auto elapsed_time = std::chrono::high_resolution_clock::now() - std::chrono::high_resolution_clock::time_point;
            TimePoint = std::chrono::high_resolution_clock::now();

            while (elapsed_time > m_cpuCycleDuration)
            {
                m_ppu.step();
                m_ppu.step();
                m_ppu.step();

                m_cpu.step();

                elapsed_time -= m_cpuCycleDuration;
            }
        }
    }

}