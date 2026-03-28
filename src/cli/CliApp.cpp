#include "cli/CliApp.h"

#include "core/DataStore.h"
#include "core/ITransport.h"
#include "core/Protocol.h"
#include "core/TransportFactory.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

using namespace embview::cli;

namespace
{
std::atomic<bool> g_running{true};

void signalHandler(int)
{
    g_running = false;
}
} // namespace

int CliApp::run(const std::vector<std::string>& args)
{
    std::string port;
    int baud = 115200;
    double duration = 0.0; // 0 = infinite
    bool jsonOutput = false;

    try
    {
        for (std::size_t i = 0; i < args.size(); ++i)
        {
            if ((args[i] == "--port" || args[i] == "-p") && i + 1 < args.size())
            {
                port = args[++i];
            }
            else if ((args[i] == "--baud" || args[i] == "-b") && i + 1 < args.size())
            {
                baud = std::stoi(args[++i]);
            }
            else if ((args[i] == "--duration" || args[i] == "-d") && i + 1 < args.size())
            {
                duration = std::stod(args[++i]);
            }
            else if (args[i] == "--json" || args[i] == "-j")
            {
                jsonOutput = true;
            }
            else if (args[i] == "--help" || args[i] == "-h")
            {
                printUsage();
                return 0;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error parsing arguments: " << e.what() << "\n";
        printUsage();
        return 1;
    }

    if (port.empty())
    {
        std::cerr << "Error: --port is required in CLI mode\n";
        printUsage();
        return 1;
    }

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    nlohmann::json config;
    config["port"] = port;
    config["baud"] = baud;

    std::unique_ptr<core::ITransport> transport;
    try
    {
        transport = core::TransportFactory::instance().create("serial", config);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error creating transport: " << e.what() << "\n";
        return 1;
    }

    if (!transport->open())
    {
        std::cerr << "Error: Failed to open " << port << "\n";
        return 1;
    }

    core::Protocol protocol;
    auto startTime = std::chrono::steady_clock::now();

    std::cout << "Listening on " << port << " at " << baud << " baud...\n";

    while (g_running)
    {
        try
        {
            if (duration > 0.0)
            {
                auto elapsed = std::chrono::steady_clock::now() - startTime;
                auto seconds = std::chrono::duration<double>(elapsed).count();
                if (seconds >= duration)
                {
                    break;
                }
            }

            auto bytes = transport->read(4096);
            if (!bytes.empty())
            {
                protocol.feedData(bytes);
                while (auto frame = protocol.parseNext())
                {
                    if (jsonOutput)
                    {
                        nlohmann::json j;
                        j["channel"] = frame->channel;
                        j["timestamp"] = frame->timestamp;
                        j["value"] = frame->value;
                        std::cout << j.dump() << "\n";
                    }
                    else
                    {
                        std::cout << "ch=" << static_cast<int>(frame->channel)
                                  << " t=" << frame->timestamp
                                  << " v=" << frame->value << "\n";
                    }
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << "\n";
            spdlog::error("CLI read error: {}", e.what());
        }
    }

    transport->close();
    std::cout << "Done.\n";
    return 0;
}

void CliApp::printUsage() const
{
    std::cout <<
        "embview CLI mode\n"
        "Usage: embview --cli [options]\n"
        "\n"
        "Options:\n"
        "  -p, --port <name>     Serial port (required)\n"
        "  -b, --baud <rate>     Baud rate (default: 115200)\n"
        "  -d, --duration <sec>  Capture duration in seconds (0 = infinite)\n"
        "  -j, --json            Output frames as JSON lines\n"
        "  -h, --help            Show this help message\n";
}
