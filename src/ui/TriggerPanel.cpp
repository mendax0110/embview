#include "ui/TriggerPanel.h"

#include "core/DataStore.h"
#include "core/TriggerEngine.h"

#include <imgui.h>

#include <algorithm>

using namespace embview::ui;

static const char* CONDITION_NAMES[] = {"Above", "Below", "Equal", "Rising Edge", "Falling Edge"};

TriggerPanel::TriggerPanel(std::shared_ptr<core::TriggerEngine> engine,
                           std::shared_ptr<core::DataStore> dataStore)
    : m_engine(std::move(engine))
    , m_dataStore(std::move(dataStore))
{
}

TriggerPanel::~TriggerPanel() = default;

void TriggerPanel::render(bool& open)
{
    if (!ImGui::Begin("Triggers", &open))
    {
        ImGui::End();
        return;
    }

    // Add trigger
    if (ImGui::Button("Add Trigger"))
    {
        core::TriggerConfig cfg;
        auto channels = m_dataStore->getActiveChannels();
        if (!channels.empty())
        {
            cfg.channel = channels.front();
        }
        m_engine->addTrigger(std::move(cfg));
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Events"))
    {
        m_engine->clearEvents();
    }

    ImGui::Separator();

    // Edit triggers
    auto& triggers = m_engine->triggers();
    int toRemove = -1;

    for (std::size_t i = 0; i < triggers.size(); ++i)
    {
        auto& t = triggers[i];
        ImGui::PushID(static_cast<int>(i));

        ImGui::Checkbox("##en", &t.enabled);
        ImGui::SameLine();

        char label[64];
        std::snprintf(label, sizeof(label), "ch %d", static_cast<int>(t.channel));
        ImGui::SetNextItemWidth(80.0f);

        int ch = static_cast<int>(t.channel);
        if (ImGui::InputInt("##ch", &ch, 1, 10))
        {
            t.channel = static_cast<uint16_t>(std::clamp(ch, 0, 65535));
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        int cond = static_cast<int>(t.condition);
        if (ImGui::Combo("##cond", &cond, CONDITION_NAMES, 5))
        {
            t.condition = static_cast<core::TriggerCondition>(cond);
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        ImGui::InputDouble("##thresh", &t.threshold, 0.1, 1.0, "%.4f");

        ImGui::SameLine();
        if (ImGui::SmallButton("X"))
        {
            toRemove = static_cast<int>(i);
        }

        ImGui::PopID();
    }

    if (toRemove >= 0)
    {
        m_engine->removeTrigger(static_cast<std::size_t>(toRemove));
    }

    // Event log
    ImGui::Spacing();
    ImGui::Text("Recent Events:");
    ImGui::Separator();

    auto events = m_engine->recentEvents(50);
    ImGui::BeginChild("TriggerEvents", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
    for (auto it = events.rbegin(); it != events.rend(); ++it)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "%s", it->message.c_str());
    }
    ImGui::EndChild();

    ImGui::End();
}
