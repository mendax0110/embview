#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace embview::core
{
    /**
     * @brief Simple math expression evaluator with channel variable support.
     *
     * Supports: +, -, *, /, parentheses, unary minus.
     * Functions: abs, sqrt, sin, cos, log, pow, min, max.
     * Variables: ch0, ch1, ... (latest channel values).
     * Constants: pi, e.
     *
     * Example: "ch0 * 3.3 / 4096" to convert ADC to voltage.
     */
    class ExpressionEval
    {
    public:
        void setVariable(const std::string& name, double value);

        void setChannelValue(uint16_t channel, double value);

        double evaluate(const std::string& expr);

        bool hasError() const;

        const std::string& errorMessage() const;

    private:
        double parseExpression();
        double parseTerm();
        double parseUnary();
        double parsePrimary();

        static double callFunction(const std::string& name, double arg);

        std::unordered_map<std::string, double> m_variables;
        std::string m_expr;
        std::size_t m_pos = 0;
        bool m_error = false;
        std::string m_errorMsg;
    };
} // namespace embview::core
