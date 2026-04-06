# --- Core library (no UI dependencies) ---
add_library(embview_core STATIC
    core/SerialTransport.cpp
    core/TcpTransport.cpp
    core/UdpTransport.cpp
    core/TransportFactory.cpp
    core/Protocol.cpp
    core/IProtocolParser.cpp
    core/DataStore.cpp
    core/DeviceManager.cpp
    core/DiagnosticRegistry.cpp
    core/LogFileManager.cpp
    core/RawDataBuffer.cpp
    core/SessionRecorder.cpp
    core/TriggerEngine.cpp
    core/ExpressionEval.cpp
    core/ConfigManager.cpp
)

target_include_directories(embview_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(embview_core PUBLIC
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    serialport
)

if(WIN32)
    target_link_libraries(embview_core PUBLIC ws2_32)
endif()

# --- UI library ---
add_library(embview_ui STATIC
    ui/Application.cpp
    ui/MainWindow.cpp
    ui/AppTheme.cpp
    ui/ConnectionPanel.cpp
    ui/PlotPanel.cpp
    ui/LogPanel.cpp
    ui/StatsPanel.cpp
    ui/CommandPanel.cpp
    ui/NumberConverterPanel.cpp
    ui/LogFilePanel.cpp
    ui/HexViewPanel.cpp
    ui/RecorderPanel.cpp
    ui/TriggerPanel.cpp
    ui/FftPanel.cpp
)

target_include_directories(embview_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(embview_ui PUBLIC
    embview_core
    imgui_lib
    implot_lib
)

# --- Executable ---
add_executable(embview
    main.cpp
    cli/CliApp.cpp
)

target_link_libraries(embview PRIVATE
    embview_core
    embview_ui
)
