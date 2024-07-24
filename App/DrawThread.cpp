#include "DrawThread.h"
#include "GuiMain.h"
#include "../../shared/ImGuiSrc/imgui.h"
#include <string>
#include <iostream>



// Custom InputText for RTL
bool InputTextRTL(const char* label, char* buf, size_t buf_size) {
    std::string reversed = reverse_utf8(buf);
    bool result = ImGui::InputText(label, &reversed[0], buf_size);
    if (result) {
        std::string reversed_back = reverse_utf8(reversed);
        strncpy_s(buf, buf_size, reversed_back.c_str(), _TRUNCATE);
    }
    return result;
}

void DrawAppWindow(void* common_ptr) {
    std::cout << "Draw thread running" << std::endl;
    auto common = (Common*)common_ptr;

    // Push style colors for transparency and custom colors
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));      // Window background color (transparent)
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));        // Border color (transparent)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.0f, 0.0f, 0.5f));        // Button background color (red)
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // Button hover color (darker red)
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));  // Button active color (dark red)

    // Title bar colors
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));         // Title bar color (transparent)
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));  // Active title bar color (transparent)
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Collapsed title bar color (transparent)

    ImGui::Begin("Alert's History", nullptr);
    ImGui::Text("Search:"); // TODO in search bar when you press refresh after search - hes updating isnt good

    static char buff[200] = "";
    static std::string search_text;  // Variable to store the input text
    static std::string active_search = "";

    // Custom style for InputText
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));       // InputText background color (transparent)
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));        // InputText border color (white)
    if (ImGui::InputText("##search", buff, sizeof(buff))) {
        search_text = buff;  // Save the text from the input box into the variable
    }
    ImGui::PopStyleColor(2); // Restore InputText style colors

    ImGui::SameLine();
    if (ImGui::Button("Search")) {
        active_search = search_text;
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        active_search = "";
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        common->data_ready = false;
        common->Alerts.clear();
        common->cv.notify_all();
    }

    if (common->data_ready) {
        if (ImGui::BeginTable("Alerts", 4)) {
            ImGui::TableSetupColumn("Date and time");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Location");
            ImGui::TableSetupColumn("Category");
            ImGui::TableHeadersRow();

            std::string reversed;
            for (auto& rec : common->Alerts) {
                if (rec.data.find(active_search) != std::string::npos) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text(rec.alertDate.c_str());
                    ImGui::TableSetColumnIndex(1);
                    reversed = reverse_utf8(rec.title);
                    ImGui::Text(reversed.c_str());
                    ImGui::TableSetColumnIndex(2);
                    reversed = reverse_utf8(rec.data);
                    ImGui::Text(reversed.c_str());
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", rec.category);
                }
            }
            ImGui::EndTable();
        }
    }
    else {
        ImGui::Text("Getting Data...");
    }
    ImGui::End();

    // Pop style colors to restore previous colors
    ImGui::PopStyleColor(8); // Restore window, border, button, and title bar colors
}

void DrawThread::operator()(Common& common) {
    GuiMain(DrawAppWindow, &common);
    std::cout << "Gui Main Finished!!" << std::endl;
    common.exit_flag = true;
}
