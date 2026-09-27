#pragma once

#include <cstdint>
#include <memory>
#include <vector>

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief Data Store.
     * \class DataStore
     */
    class DataStore;
} // namespace embview::core

/**
 * @namespace embview::ui Ui support.
 */
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
        /**
         * @brief Construct the FFT panel.
         * @param dataStore Shared data store used for FFT input.
         */
        explicit FftPanel(std::shared_ptr<core::DataStore> dataStore);

        /**
         * @brief Destroy the FFT panel.
         */
        ~FftPanel();

        /**
         * @brief Render the FFT panel.
         * @param open Whether the panel is open.
         */
        void render(bool& open);

    private:
        /**
         * @brief Compute the FFT of the provided real/imag arrays.
         * @param real Real-valued input data.
         * @param imag Imaginary-valued input data.
         */
        static void fft(std::vector<double>& real, std::vector<double>& imag);

        /**
         * @brief Round @p n up to the next power of two.
         * @param n Value to round up.
         * @return Next power-of-two size.
         */
        static std::size_t nextPow2(std::size_t n);

        std::shared_ptr<core::DataStore> m_dataStore;
        uint16_t m_selectedChannel = 0;
        int m_fftSize = 1024;
        double m_sampleRate = 1000.0;
        std::vector<double> m_magnitudes;
        std::vector<double> m_frequencies;
    };
} // namespace embview::ui
