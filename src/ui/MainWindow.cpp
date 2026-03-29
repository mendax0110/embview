#include "ui/MainWindow.h"

#include "core/ConfigManager.h"
#include "core/DataStore.h"
#include "core/DeviceManager.h"
#include "core/ExpressionEval.h"
#include "core/LogFileManager.h"
#include "core/RawDataBuffer.h"
#include "core/SessionRecorder.h"
#include "core/TriggerEngine.h"
#include "ui/CommandPanel.h"
#include "ui/ConnectionPanel.h"
#include "ui/FftPanel.h"
#include "ui/HexViewPanel.h"
#include "ui/LogFilePanel.h"
#include "ui/LogPanel.h"
#include "ui/NumberConverterPanel.h"
#include "ui/PlotPanel.h"
#include "ui/RecorderPanel.h"
#include "ui/StatsPanel.h"
#include "ui/TriggerPanel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <algorithm>

using namespace embview::ui;
using namespace embview::core;

MainWindow::MainWindow(std::shared_ptr<core::DataStore> dataStore,
                       std::shared_ptr<core::DeviceManager> deviceMgr,
                       std::shared_ptr<core::LogFileManager> logFileMgr)
    : m_dataStore(std::move(dataStore))
    , m_deviceMgr(std::move(deviceMgr))
    , m_triggerEngine(std::make_shared<TriggerEngine>())
    , m_exprEval(std::make_shared<ExpressionEval>())
    , m_recorder(std::make_shared<SessionRecorder>())
    , m_configMgr(std::make_shared<ConfigManager>())
{
    m_configMgr->load();

    m_connectionPanel = std::make_unique<ConnectionPanel>(m_deviceMgr);
    m_plotPanel = std::make_unique<PlotPanel>(m_dataStore, m_triggerEngine, m_exprEval, m_recorder);
    m_logPanel = std::make_unique<LogPanel>();
    m_statsPanel = std::make_unique<StatsPanel>(m_dataStore);
    m_commandPanel = std::make_unique<CommandPanel>(m_deviceMgr);
    m_numberConverterPanel = std::make_unique<NumberConverterPanel>();
    m_logFilePanel = std::make_unique<LogFilePanel>(std::move(logFileMgr));
    m_hexViewPanel = std::make_unique<HexViewPanel>(m_deviceMgr->getRawDataBuffer());
    m_recorderPanel = std::make_unique<RecorderPanel>(m_dataStore, m_recorder);
    m_triggerPanel = std::make_unique<TriggerPanel>(m_triggerEngine, m_dataStore);
    m_fftPanel = std::make_unique<FftPanel>(m_dataStore);
}

MainWindow::~MainWindow()
{
    m_configMgr->save();
}

void MainWindow::render()
{
    renderDockSpace();
    renderMenuBar();

    if (m_showConnectionPanel)  m_connectionPanel->render(m_showConnectionPanel);
    if (m_showPlotPanel)        m_plotPanel->render(m_showPlotPanel);
    if (m_showLogPanel)         m_logPanel->render(m_showLogPanel);
    if (m_showStatsPanel)       m_statsPanel->render(m_showStatsPanel);
    if (m_showCommandPanel)     m_commandPanel->render(m_showCommandPanel);
    if (m_showNumberConverter)  m_numberConverterPanel->render(m_showNumberConverter);
    if (m_showLogFilePanel)     m_logFilePanel->render(m_showLogFilePanel);
    if (m_showHexView)          m_hexViewPanel->render(m_showHexView);
    if (m_showRecorder)         m_recorderPanel->render(m_showRecorder);
    if (m_showTriggers)         m_triggerPanel->render(m_showTriggers);
    if (m_showFft)              m_fftPanel->render(m_showFft);

    renderStatusBar();
}

bool MainWindow::shouldClose() const
{
    return m_shouldClose;
}

void MainWindow::requestLayoutRebuild()
{
    m_needsLayoutRebuild = true;
}

void MainWindow::renderMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Config"))
            {
                m_configMgr->save();
            }
            if (ImGui::MenuItem("Load Config"))
            {
                m_configMgr->load();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                m_shouldClose = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Connection", nullptr, &m_showConnectionPanel);
            ImGui::MenuItem("Plot", nullptr, &m_showPlotPanel);
            ImGui::MenuItem("Statistics", nullptr, &m_showStatsPanel);
            ImGui::MenuItem("Log", nullptr, &m_showLogPanel);
            ImGui::MenuItem("Log Files", nullptr, &m_showLogFilePanel);
            ImGui::MenuItem("Command Builder", nullptr, &m_showCommandPanel);
            ImGui::MenuItem("Number Converter", nullptr, &m_showNumberConverter);
            ImGui::Separator();
            ImGui::MenuItem("Hex View", nullptr, &m_showHexView);
            ImGui::MenuItem("Recorder", nullptr, &m_showRecorder);
            ImGui::MenuItem("Triggers", nullptr, &m_showTriggers);
            ImGui::MenuItem("FFT Analysis", nullptr, &m_showFft);
            ImGui::Separator();
            ImGui::MenuItem("Auto-Scale Layout", nullptr, &m_autoScaleLayout);
            if (ImGui::MenuItem("Reset Layout"))
            {
                m_needsLayoutRebuild = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                m_showAboutPopup = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (m_showAboutPopup)
    {
        ImGui::OpenPopup("About embview");
        m_showAboutPopup = false;
    }

    if (ImGui::BeginPopupModal("About embview", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("embview v0.3.0");
        ImGui::Separator();
        ImGui::Text("Embedded Microcontroller Analysis Tool");
        ImGui::Text("Serial / TCP / UDP multi-device support");
        ImGui::Text("Built with Dear ImGui and ImPlot");
        ImGui::Spacing();

        ImGui::Separator();
        ImGui::Text("Author: mendax0110");
        ImGui::Spacing();

        ImGui::Text("Website:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "mendax0110.github.io/terminal/");
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        if (ImGui::IsItemClicked())
        {
#ifdef _WIN32
            system("start https://mendax0110.github.io/terminal/");
#else
            system("xdg-open https://mendax0110.github.io/terminal/");
#endif
        }

        ImGui::Text("GitHub:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "github.com/mendax0110");
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        if (ImGui::IsItemClicked())
        {
#ifdef _WIN32
            system("start https://github.com/mendax0110/");
#else
            system("xdg-open https://github.com/mendax0110/");
#endif
        }

        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("Close", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void MainWindow::renderStatusBar()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float height = ImGui::GetFrameHeight();

    if (viewport->WorkSize.x < 1.0f || viewport->WorkSize.y < height + 1.0f)
    {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - height));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, height));
    ImGui::SetNextWindowViewport(viewport->ID);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoInputs;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));

    ImGui::Begin("##StatusBar", nullptr, flags);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    const auto deviceCount = m_deviceMgr->deviceCount();
    if (deviceCount > 0)
    {
        ImGui::TextColored(ImVec4(0.3f, 0.85f, 0.45f, 1.0f), "CONNECTED (%d)", static_cast<int>(deviceCount));
    }
    else
    {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "DISCONNECTED");
    }

    ImGui::SameLine();
    const auto channels = m_dataStore->getActiveChannels();
    ImGui::Text("| Channels: %d", static_cast<int>(channels.size()));

    const double currentTime = ImGui::GetTime();
    std::size_t totalFrames = 0;
    for (const auto ch : channels)
    {
        totalFrames += m_dataStore->getChannelSize(ch);
    }

    if (currentTime - m_lastFpsTime >= 1.0)
    {
        m_dataFps = static_cast<float>(totalFrames - m_lastFrameCount) / static_cast<float>(currentTime - m_lastFpsTime);
        m_lastFrameCount = totalFrames;
        m_lastFpsTime = currentTime;
    }

    ImGui::SameLine();
    ImGui::Text("| Data: %.1f fps", m_dataFps);

    ImGui::SameLine();
    ImGui::Text("| FPS: %.0f", ImGui::GetIO().Framerate);

    if (m_recorder->isRecording())
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "| REC");
    }

    ImGui::End();
}

void MainWindow::renderDockSpace()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float statusBarHeight = ImGui::GetFrameHeight();

    const float dockW = viewport->WorkSize.x;
    const float dockH = viewport->WorkSize.y - statusBarHeight;

    if (dockW < 1.0f || dockH < 1.0f)
    {
        return;
    }

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(dockW, dockH));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    const ImGuiID dockspaceId = ImGui::GetID("MainDockSpace");

    const float currentW = dockW;
    const float currentH = dockH;
    if (m_autoScaleLayout && m_lastLayoutWidth > 0.0f)
    {
        const float widthRatio = currentW / m_lastLayoutWidth;
        const float heightRatio = currentH / m_lastLayoutHeight;
        if (widthRatio < 0.75f || widthRatio > 1.33f || heightRatio < 0.75f || heightRatio > 1.33f)
        {
            m_needsLayoutRebuild = true;
        }
    }

    if (m_needsLayoutRebuild)
    {
        if (currentW > 1.0f && currentH > 1.0f)
        {
            buildDefaultLayout(dockspaceId);
            m_needsLayoutRebuild = false;
            m_lastLayoutWidth = currentW;
            m_lastLayoutHeight = currentH;
        }
    }

    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();
}

void MainWindow::buildDefaultLayout(const ImGuiID dockspaceId)
{
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float statusBarHeight = ImGui::GetFrameHeight();
    float w = viewport->WorkSize.x;
    float h = viewport->WorkSize.y - statusBarHeight;

    w = std::max(w, 1.0f);
    h = std::max(h, 1.0f);

    ImGui::DockBuilderSetNodeSize(dockspaceId, ImVec2(w, h));

    ImGuiID dockLeft;
    ImGuiID dockRight;
    ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.22f, &dockLeft, &dockRight);

    ImGuiID dockCenter;
    ImGuiID dockBottom;
    ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.30f, &dockBottom, &dockCenter);

    ImGui::DockBuilderDockWindow("Connection", dockLeft);
    ImGui::DockBuilderDockWindow("Command Builder", dockLeft);
    ImGui::DockBuilderDockWindow("Recorder", dockLeft);
    ImGui::DockBuilderDockWindow("Triggers", dockLeft);

    ImGui::DockBuilderDockWindow("Plot", dockCenter);
    ImGui::DockBuilderDockWindow("FFT Analysis", dockCenter);

    ImGui::DockBuilderDockWindow("Log", dockBottom);
    ImGui::DockBuilderDockWindow("Statistics", dockBottom);
    ImGui::DockBuilderDockWindow("Log Files", dockBottom);
    ImGui::DockBuilderDockWindow("Number Converter", dockBottom);
    ImGui::DockBuilderDockWindow("Hex View", dockBottom);

    ImGui::DockBuilderFinish(dockspaceId);
}
