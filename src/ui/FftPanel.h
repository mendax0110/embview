#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace embview::core
{
    class DataStore;
} // namespace embview::core

namespace embview::ui
{
    /**
     * @brief Panel showing FFT frequency analysis of channel data.
     *
     * Uses a radix-2 Cooley-Tukey FFT on a power-of-2 windowed sample.
     */
    class FftPanel
    {
    public:
        explicit FftPanel(std::shared_ptr<core::DataStore> dataStore);
        ~FftPanel();

        void render(bool& open);

    private:
        static void fft(std::vector<double>& real, std::vector<double>& imag);
        static std::size_t nextPow2(std::size_t n);

        std::shared_ptr<core::DataStore> m_dataStore;
        uint16_t m_selectedChannel = 0;
        int m_fftSize = 1024;
        double m_sampleRate = 1000.0;
        std::vector<double> m_magnitudes;
        std::vector<double> m_frequencies;
    };
} // namespace embview::ui
