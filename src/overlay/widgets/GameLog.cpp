#include <stdafx.h>

#include "GameLog.h"

#include <Utils.h>

GameLog::GameLog()
{
    const auto gamelogSink = CreateCustomSinkST([this](const std::string& msg) { Log(msg); });
    gamelogSink->set_pattern("%v");
    spdlog::get("gamelog")->sinks().push_back(gamelogSink);
}

WidgetResult GameLog::OnEnable()
{
    return WidgetResult::ENABLED;
}

WidgetResult GameLog::OnDisable()
{
    return WidgetResult::DISABLED;
}

void GameLog::Update()
{
    const auto itemWidth = GetAlignedItemWidth(2);

    if (ImGui::Button("Clear output", ImVec2(itemWidth, 0)))
    {
        std::lock_guard _{ m_gamelogLock };
        m_gamelogLines.clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_gamelogShouldScroll);

    if (ImGui::ListBoxHeader("##GameLogHeader", ImVec2(-1, -1)))
    {
        std::lock_guard _{ m_gamelogLock };

        ImGuiListClipper clipper;
        clipper.Begin(m_gamelogLines.size());
        while (clipper.Step())
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                auto& item = m_gamelogLines[i];
                ImGui::PushID(i);
                ImGui::Selectable(item.c_str());
                ImGui::PopID();
            }

        if (m_gamelogScroll)
        {
            if (m_gamelogShouldScroll)
                ImGui::SetScrollHereY();
            m_gamelogScroll = false;
        }

        ImGui::ListBoxFooter();
    }
}

void GameLog::Log(const std::string& acpText)
{
    std::lock_guard _{ m_gamelogLock };

    size_t first = 0;
    size_t size = acpText.size();
    while (first < size)
    {
        // find_first_of \r or \n
        size_t second = std::string::npos;
        for (size_t i = first; i != size; ++i)
        {
            char ch = acpText[i];
            if (ch == '\r' || ch == '\n')
            {
                second = i;
                break;
            }
        }

        if (second == std::string_view::npos)
        {
            m_gamelogLines.emplace_back(acpText.substr(first));
            break;
        }

        if (first != second)
            m_gamelogLines.emplace_back(acpText.substr(first, second-first));

        first = second + 1;
        char ch = acpText[first];
        while (ch == '\r' || ch == '\n')
            ch = acpText[++first];
    }

    m_gamelogScroll = true;
}
