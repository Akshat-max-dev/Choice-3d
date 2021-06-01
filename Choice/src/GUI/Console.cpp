#include "Console.h"

#include <imgui.h>

#include "IconsFontAwesome5.h"
#include "Choice.h"

namespace choice
{
	Console::Console()
	{

	}

	Console::~Console()
	{
		Messages.clear();
	}

	void Console::Execute()
	{
		ImGui::Begin(ICON_FA_TERMINAL "Console");

		if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered())
		{
			Choice::Instance()->GetEditor()->GetCamera()->AcceptInput(false);
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.2f, 0.6f, 0.8f, 1.0f });
		if (ImGui::Button(ICON_FA_SYNC_ALT))
		{
			Messages.clear();
		}
		ImGui::PopStyleColor(2);

		ImGui::SameLine();

		ImGui::Text("Show Messages ");
		ImGui::SameLine();

		static const char* comboitems[4] = { "All", "Error", "Warning", "Info" };
		static int currentItem = 0;
		ImGui::SetNextItemWidth(150.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
		ImGui::Combo("##Show", &currentItem, comboitems, 4);
		ImGui::PopStyleVar(2);

		ImGui::Separator();

		ImGui::BeginChild("##Output", ImGui::GetContentRegionAvail());

		for (auto& message : Messages)
		{
			std::string icon = "";

			switch (message.first)
			{
			case MESSAGE_TYPE::ERROR_MSG:
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.0f, 0.0f, 1.0f });
				icon = ICON_FA_BUG;
				break;
			case MESSAGE_TYPE::WARNING:
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 0.0f, 1.0f });
				icon = ICON_FA_INFO;
				break;
			case MESSAGE_TYPE::INFO:
				ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 1.0f, 0.0f, 1.0f });
				icon = ICON_FA_INFO;
				break;
			default:
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });
				icon = ICON_FA_INFO;
				break;
			}

			std::string msg;
			switch (currentItem)
			{
			case 1: 
				if (message.first == MESSAGE_TYPE::ERROR_MSG) 
				{ 
					msg = message.second; 
					msg = msg.substr(0, msg.find_last_of('{'));
					ImGui::TextUnformatted((icon + " " + msg).c_str());
				} 
				break;
			case 2: 
				if (message.first == MESSAGE_TYPE::WARNING) 
				{ 
					msg = message.second;
					msg = msg.substr(0, msg.find_last_of('{'));
					ImGui::TextUnformatted((icon + " " + msg).c_str());
				} 
				break;
			case 3: 
				if (message.first == MESSAGE_TYPE::INFO) 
				{ 
					msg = message.second; 
					msg = msg.substr(0, msg.find_last_of('{'));
					ImGui::TextUnformatted((icon + " " + msg).c_str());
				} 
				break;
			default: 
				msg = message.second; 
				msg = msg.substr(0, msg.find_last_of('{'));
				ImGui::TextUnformatted((icon + " " + msg).c_str());
				break;
			}

			ImGui::PopStyleColor();
		}

		ImGui::EndChild();

		ImGui::End();
	}
}