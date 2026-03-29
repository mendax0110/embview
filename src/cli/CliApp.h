#pragma once

#include <string>
#include <vector>

/// @brief Command-line interface application for serial data capture and display. \namespace embview::cli
namespace embview::cli
{
    /**
    * @brief Command-line interface for serial data capture and display. \class CliApp
    *
    * Reads frames from a serial port and prints parsed data to stdout.
    * Runs until the requested duration expires or the user interrupts with Ctrl+C.
    */
    class CliApp
    {
    public:

        /**
         * @brief Run the CLI application with the given command-line arguments.
         *
         * Supported options:
         *   -p, --port <name>     Serial port (required)
         *   -b, --baud <rate>     Baud rate (default: 115200)
         *   -d, --duration <sec> Capture duration in seconds (0 = infinite)
         *   -j, --json            Output frames as JSON lines
         *
         * @param args Command-line arguments (excluding program name).
         * @return Exit code (0 on success).
         */
        static int run(const std::vector<std::string>& args);

    private:

        /**
         * @brief Print usage information for the CLI application.
         */
        static void printUsage();
    };
} // namespace embview::cli
