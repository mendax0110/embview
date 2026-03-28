#include "ui/FftPanel.h"

#include "core/DataStore.h"

#include <imgui.h>
#include <implot.h>

#include <algorithm>
#include <cmath>
#include <numbers>

using namespace embview::ui;

FftPanel::FftPanel(std::shared_ptr<core::DataStore> dataStore)
    : m_dataStore(std::move(dataStore))
{
}

FftPanel::~FftPanel() = default;

std::size_t FftPanel::nextPow2(std::size_t n)
{
    std::size_t p = 1;
    while (p < n)
    {
        p <<= 1;
    }
    return p;
}

void FftPanel::fft(std::vector<double>& real, std::vector<double>& imag)
{
    std::size_t n = real.size();
    if (n <= 1)
    {
        return;
    }

    // Bit-reversal permutation
    for (std::size_t i = 1, j = 0; i < n; ++i)
    {
        std::size_t bit = n >> 1;
        while (j & bit)
        {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;

        if (i < j)
        {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    // Cooley-Tukey
    for (std::size_t len = 2; len <= n; len <<= 1)
    {
        double angle = -2.0 * std::numbers::pi / static_cast<double>(len);
        double wReal = std::cos(angle);
        double wImag = std::sin(angle);

        for (std::size_t i = 0; i < n; i += len)
        {
            double curReal = 1.0;
            double curImag = 0.0;
            std::size_t half = len / 2;

            for (std::size_t j = 0; j < half; ++j)
            {
                double tReal = curReal * real[i + j + half] - curImag * imag[i + j + half];
                double tImag = curReal * imag[i + j + half] + curImag * real[i + j + half];

                real[i + j + half] = real[i + j] - tReal;
                imag[i + j + half] = imag[i + j] - tImag;
                real[i + j] += tReal;
                imag[i + j] += tImag;

                double newReal = curReal * wReal - curImag * wImag;
                curImag = curReal * wImag + curImag * wReal;
                curReal = newReal;
            }
        }
    }
}

void FftPanel::render(bool& open)
{
    if (!ImGui::Begin("FFT Analysis", &open))
    {
        ImGui::End();
        return;
    }

    auto channels = m_dataStore->getActiveChannels();
    std::sort(channels.begin(), channels.end());

    if (channels.empty())
    {
        ImGui::TextDisabled("No channels available -- connect a device first");
        ImGui::End();
        return;
    }

    // Channel selector
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::BeginCombo("Channel", std::to_string(m_selectedChannel).c_str()))
    {
        for (uint16_t ch : channels)
        {
            bool selected = (ch == m_selectedChannel);
            if (ImGui::Selectable(std::to_string(ch).c_str(), selected))
            {
                m_selectedChannel = ch;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputDouble("Sample Rate (Hz)", &m_sampleRate, 100.0, 1000.0, "%.0f");
    m_sampleRate = std::max(m_sampleRate, 1.0);

    ImGui::SameLine();
    if (ImGui::Button("Compute FFT"))
    {
        auto frames = m_dataStore->getChannel(m_selectedChannel);
        if (!frames.empty())
        {
            std::size_t n = nextPow2(frames.size());
            n = std::min(n, static_cast<std::size_t>(8192)); // cap

            std::vector<double> real(n, 0.0);
            std::vector<double> imag(n, 0.0);

            std::size_t copyCount = std::min(frames.size(), n);
            for (std::size_t i = 0; i < copyCount; ++i)
            {
                real[i] = frames[frames.size() - copyCount + i].value;
            }

            // Apply Hanning window
            for (std::size_t i = 0; i < copyCount; ++i)
            {
                double w = 0.5 * (1.0 - std::cos(2.0 * std::numbers::pi * static_cast<double>(i)
                                                    / static_cast<double>(copyCount - 1)));
                real[i] *= w;
            }

            fft(real, imag);

            std::size_t halfN = n / 2;
            m_magnitudes.resize(halfN);
            m_frequencies.resize(halfN);

            double freqStep = m_sampleRate / static_cast<double>(n);
            for (std::size_t i = 0; i < halfN; ++i)
            {
                m_magnitudes[i] = 2.0 * std::sqrt(real[i] * real[i] + imag[i] * imag[i])
                                  / static_cast<double>(n);
                m_frequencies[i] = static_cast<double>(i) * freqStep;
            }
        }
    }

    // Plot
    if (!m_magnitudes.empty() && ImPlot::BeginPlot("Frequency Spectrum", ImVec2(-1, -1)))
    {
        ImPlot::SetupAxes("Frequency (Hz)", "Magnitude");
        ImPlot::PlotBars("FFT", m_frequencies.data(), m_magnitudes.data(),
                          static_cast<int>(m_magnitudes.size()),
                          m_frequencies.size() > 1 ? m_frequencies[1] - m_frequencies[0] : 1.0);
        ImPlot::EndPlot();
    }

    ImGui::End();
}
