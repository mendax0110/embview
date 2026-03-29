#include "cli/CliApp.h"
#include "core/DiagnosticRegistry.h"
#include "core/LogFileManager.h"
#include "ui/Application.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    try
    {
        spdlog::set_level(spdlog::level::info);

        const auto logFileMgr = std::make_shared<embview::core::LogFileManager>();
        logFileMgr->init();

        std::vector<std::string> args(argv + 1, argv + argc);

        const bool cliMode = std::ranges::find(args.begin(), args.end(), "--cli") != args.end();

        int result = 0;

        if (cliMode)
        {
            args.erase(std::ranges::remove(args, "--cli").begin(), args.end());

            embview::cli::CliApp cli;
            result = cli.run(args);
        }
        else
        {
            embview::ui::Application app;
            if (!app.init(logFileMgr))
            {
                spdlog::error("Failed to initialize application");
                result = 1;
            }
            else
            {
                app.run();
                app.shutdown();
            }
        }

        embview::core::DiagnosticRegistry::instance().dumpToLog();
        return result;
    }
    catch (const std::exception& e)
    {
        spdlog::critical("Fatal error: {}", e.what());
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
