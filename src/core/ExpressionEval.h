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
        /**
         * @brief Setter for Variable
         * @param name The name of the var
         * @param value The value of the var
         */
        void setVariable(const std::string& name, double value);

        /**
         * @brief Setter for the channel value
         * @param channel The channel number
         * @param value The value of the channel
         */
        void setChannelValue(uint16_t channel, double value);

        /**
         * @brief Helper to evaluate a given string
         * @param expr The string to eval
         * @return A double
         */
        double evaluate(const std::string& expr);

        /**
         * @brief Checks if an error occurred
         * @return True if there is an errror, false otherwise
         */
        bool hasError() const;

        /**
         * @brief Getter for the error message
         * @return A String representing the error message
         */
        const std::string& errorMessage() const;

    private:
        /**
         * @brief Parses a Expression
         * @return A double
         */
        double parseExpression();

        /**
         * @brief Parses a term
         * @return A double
         */
        double parseTerm();

        /**
         * @brief Parses a Unary
         * @return A double
         */
        double parseUnary();

        /**
         * @brief Parses a primary
         * @return A double
         */
        double parsePrimary();

        /**
         * @brief Helper to call function
         * @param name The name of the fnc
         * @param arg arg
         * @return A double
         */
        static double callFunction(const std::string& name, double arg);

        std::unordered_map<std::string, double> m_variables;
        std::string m_expr;
        std::size_t m_pos = 0;
        bool m_error = false;
        std::string m_errorMsg;
    };
} // namespace embview::core
