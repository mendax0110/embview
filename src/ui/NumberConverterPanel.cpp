#include "ui/NumberConverterPanel.h"

#include <imgui.h>

#include <bit>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <string>

using namespace embview::ui;

namespace
{
    bool tryParse(const char* input, uint64_t& outVal)
    {
        std::string s(input);
        if (s.empty())
        {
            return false;
        }

        // Remove leading/trailing whitespace
        while (!s.empty() && s.front() == ' ') s.erase(s.begin());
        while (!s.empty() && s.back() == ' ') s.pop_back();

        if (s.empty())
        {
            return false;
        }

        int base = 10;
        const char* start = s.c_str();

        if (s.size() > 2 && s[0] == '0')
        {
            if (s[1] == 'x' || s[1] == 'X')
            {
                base = 16;
                start += 2;
            }
            else if (s[1] == 'b' || s[1] == 'B')
            {
                base = 2;
                start += 2;
            }
            else if (s[1] == 'o' || s[1] == 'O')
            {
                base = 8;
                start += 2;
            }
        }

        auto result = std::from_chars(start, s.c_str() + s.size(), outVal, base);
        return result.ec == std::errc{};
    }

    std::string toBinaryString(uint64_t val, int bits)
    {
        std::string result;
        for (int i = bits - 1; i >= 0; --i)
        {
            result += ((val >> i) & 1) ? '1' : '0';
            if (i > 0 && i % 8 == 0)
            {
                result += ' ';
            }
        }
        return result;
    }
} // namespace

void NumberConverterPanel::render(bool& open)
{
    if (!ImGui::Begin("Number Converter", &open))
    {
        ImGui::End();
        return;
    }

    ImGui::InputText("Input", m_input, sizeof(m_input));
    ImGui::SameLine();
    ImGui::TextDisabled("(0x=hex, 0b=bin, 0o=oct)");

    ImGui::Combo("Width", &m_bitWidth, "8-bit\0" "16-bit\0" "32-bit\0" "64-bit\0");
    ImGui::SameLine();
    ImGui::Checkbox("Signed", &m_isSigned);

    ImGui::Separator();

    uint64_t val = 0;
    if (!tryParse(m_input, val))
    {
        ImGui::TextDisabled("Enter a valid number");
        ImGui::End();
        return;
    }

    int bits = 8 << m_bitWidth; // 8, 16, 32, 64
    uint64_t mask = (bits == 64) ? ~0ULL : ((1ULL << bits) - 1);
    val &= mask;

    // Decimal display
    if (m_isSigned)
    {
        int64_t signedVal = static_cast<int64_t>(val);
        // Sign-extend if needed
        if (bits < 64 && (val & (1ULL << (bits - 1))))
        {
            signedVal = static_cast<int64_t>(val | (~mask));
        }
        ImGui::Text("Decimal:  %lld", static_cast<long long>(signedVal));
    }
    else
    {
        ImGui::Text("Decimal:  %llu", static_cast<unsigned long long>(val));
    }

    // Hex
    char hexBuf[32];
    snprintf(hexBuf, sizeof(hexBuf), "0x%0*llX", bits / 4, static_cast<unsigned long long>(val));
    ImGui::Text("Hex:      %s", hexBuf);

    // Octal
    ImGui::Text("Octal:    0o%llo", static_cast<unsigned long long>(val));

    // Binary
    std::string binStr = toBinaryString(val, bits);
    ImGui::Text("Binary:   0b%s", binStr.c_str());

    // IEEE 754 interpretation
    ImGui::Separator();
    if (bits == 32)
    {
        float f;
        uint32_t u32 = static_cast<uint32_t>(val);
        std::memcpy(&f, &u32, sizeof(float));
        ImGui::Text("Float32:  %g", static_cast<double>(f));
    }
    else if (bits == 64)
    {
        double d;
        std::memcpy(&d, &val, sizeof(double));
        ImGui::Text("Float64:  %g", d);
    }

    ImGui::End();
}
