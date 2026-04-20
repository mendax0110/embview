#include "core/ExpressionEval.h"

#include <cctype>
#include <cmath>
#include <format>
#include <numbers>
#include <stdexcept>

using namespace embview::core;

void ExpressionEval::setVariable(const std::string& name, const double value)
{
    m_variables[name] = value;
}

void ExpressionEval::setChannelValue(const uint16_t channel, const double value)
{
    m_variables["ch" + std::to_string(channel)] = value;
}

double ExpressionEval::evaluate(const std::string& expr)
{
    m_expr = expr;
    m_pos = 0;
    m_error = false;
    m_errorMsg.clear();

    try
    {
        const double result = parseExpression();

        // Check for trailing characters
        while (m_pos < m_expr.size() && std::isspace(static_cast<unsigned char>(m_expr[m_pos])))
        {
            ++m_pos;
        }
        if (m_pos < m_expr.size())
        {
            m_error = true;
            m_errorMsg = std::format("Unexpected character at position {}", m_pos);
            return 0.0;
        }

        return result;
    }
    catch (const std::exception& e)
    {
        m_error = true;
        m_errorMsg = e.what();
        return 0.0;
    }
}

bool ExpressionEval::hasError() const
{
    return m_error;
}

const std::string& ExpressionEval::errorMessage() const
{
    return m_errorMsg;
}

// expression = term (('+' | '-') term)*
double ExpressionEval::parseExpression()
{
    double left = parseTerm();

    while (m_pos < m_expr.size())
    {
        // Skip whitespace
        while (m_pos < m_expr.size() && std::isspace(static_cast<unsigned char>(m_expr[m_pos])))
        {
            ++m_pos;
        }

        if (m_pos >= m_expr.size())
        {
            break;
        }

        const char op = m_expr[m_pos];
        if (op != '+' && op != '-')
        {
            break;
        }
        ++m_pos;
        const double right = parseTerm();

        if (op == '+')
        {
            left += right;
        }
        else
        {
            left -= right;
        }
    }
    return left;
}

// term = unary (('*' | '/') unary)*
double ExpressionEval::parseTerm()
{
    double left = parseUnary();

    while (m_pos < m_expr.size())
    {
        while (m_pos < m_expr.size() && std::isspace(static_cast<unsigned char>(m_expr[m_pos])))
        {
            ++m_pos;
        }
        if (m_pos >= m_expr.size())
        {
            break;
        }

        const char op = m_expr[m_pos];
        if (op != '*' && op != '/')
        {
            break;
        }
        ++m_pos;
        const double right = parseUnary();

        if (op == '*')
        {
            left *= right;
        }
        else
        {
            if (right == 0.0)
            {
                throw std::runtime_error("Division by zero");
            }
            left /= right;
        }
    }
    return left;
}

// unary = '-' unary | primary
double ExpressionEval::parseUnary()
{
    while (m_pos < m_expr.size() && std::isspace(static_cast<unsigned char>(m_expr[m_pos])))
    {
        ++m_pos;
    }

    if (m_pos < m_expr.size() && m_expr[m_pos] == '-')
    {
        ++m_pos;
        return -parseUnary();
    }

    return parsePrimary();
}

// primary = number | '(' expression ')' | function '(' expression ')' | variable
double ExpressionEval::parsePrimary()
{
    while (m_pos < m_expr.size() && std::isspace(static_cast<unsigned char>(m_expr[m_pos])))
    {
        ++m_pos;
    }

    if (m_pos >= m_expr.size())
    {
        throw std::runtime_error("Unexpected end of expression");
    }

    // Parenthesized expression
    if (m_expr[m_pos] == '(')
    {
        ++m_pos;
        const double val = parseExpression();
        while (m_pos < m_expr.size() && std::isspace(static_cast<unsigned char>(m_expr[m_pos])))
        {
            ++m_pos;
        }
        if (m_pos >= m_expr.size() || m_expr[m_pos] != ')')
        {
            throw std::runtime_error("Missing closing parenthesis");
        }
        ++m_pos;
        return val;
    }

    // Number
    if (std::isdigit(static_cast<unsigned char>(m_expr[m_pos])) || m_expr[m_pos] == '.')
    {
        const std::size_t start = m_pos;
        while (m_pos < m_expr.size() &&
               (std::isdigit(static_cast<unsigned char>(m_expr[m_pos])) || m_expr[m_pos] == '.' ||
                m_expr[m_pos] == 'e' || m_expr[m_pos] == 'E'))
        {
            ++m_pos;
            // Handle exponent sign
            if (m_pos < m_expr.size() && (m_expr[m_pos - 1] == 'e' || m_expr[m_pos - 1] == 'E') &&
                (m_expr[m_pos] == '+' || m_expr[m_pos] == '-'))
            {
                ++m_pos;
            }
        }
        return std::stod(m_expr.substr(start, m_pos - start));
    }

    // Identifier (function or variable)
    if (std::isalpha(static_cast<unsigned char>(m_expr[m_pos])) || m_expr[m_pos] == '_')
    {
        const std::size_t start = m_pos;
        while (m_pos < m_expr.size() &&
               (std::isalnum(static_cast<unsigned char>(m_expr[m_pos])) || m_expr[m_pos] == '_'))
        {
            ++m_pos;
        }
        const std::string name = m_expr.substr(start, m_pos - start);

        // Skip whitespace
        while (m_pos < m_expr.size() && std::isspace(static_cast<unsigned char>(m_expr[m_pos])))
        {
            ++m_pos;
        }

        // Function call
        if (m_pos < m_expr.size() && m_expr[m_pos] == '(')
        {
            ++m_pos;
            const double arg = parseExpression();
            while (m_pos < m_expr.size() && std::isspace(static_cast<unsigned char>(m_expr[m_pos])))
            {
                ++m_pos;
            }
            if (m_pos >= m_expr.size() || m_expr[m_pos] != ')')
            {
                throw std::runtime_error("Missing closing parenthesis for function " + name);
            }
            ++m_pos;
            return callFunction(name, arg);
        }

        // Built-in constants
        if (name == "pi") return std::numbers::pi;
        if (name == "e") return std::numbers::e;

        // Variable lookup
        const auto it = m_variables.find(name);
        if (it != m_variables.end())
        {
            return it->second;
        }

        throw std::runtime_error("Unknown variable: " + name);
    }

    throw std::runtime_error(std::format("Unexpected character '{}' at position {}",
                                          m_expr[m_pos], m_pos));
}

double ExpressionEval::callFunction(const std::string& name, const double arg)
{
    if (name == "abs") return std::abs(arg);
    if (name == "sqrt") return std::sqrt(arg);
    if (name == "sin") return std::sin(arg);
    if (name == "cos") return std::cos(arg);
    if (name == "tan") return std::tan(arg);
    if (name == "log") return std::log(arg);
    if (name == "log10") return std::log10(arg);
    if (name == "exp") return std::exp(arg);
    if (name == "floor") return std::floor(arg);
    if (name == "ceil") return std::ceil(arg);
    if (name == "round") return std::round(arg);

    throw std::runtime_error("Unknown function: " + name);
}
