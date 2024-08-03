#pragma once
#include "DrawThread.h"

// Fade-in variables for title and subtitle
float title_alpha = 0.0f;
float subtitle_alpha = 0.0f;
const float alpha_increment = 0.009f; // Adjust the speed of the fade-in effect

// Screen dimensions
int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

/**
 * @brief Draws the main application window.
 * @param common_ptr Pointer to the Common structure containing shared data.
 */
void DrawAppWindow(void* common_ptr) {
    auto common = (Common*)common_ptr;

    // Get the ImGui IO object and retrieve the loaded fonts
    ImGuiIO& io = ImGui::GetIO();
    static ImFont* titleFont = io.Fonts->Fonts[1];
    static ImFont* subtitleFont = io.Fonts->Fonts[2];
    static ImFont* clockFont = io.Fonts->Fonts[3];

    // Display various UI elements
    AlertCount(common_ptr, clockFont);
    Clock(clockFont);

    static bool show_history = false;
    if (!show_history) {
        title(titleFont, subtitleFont);
    }

    LiveAlertsWindow(common_ptr);
    menueButtons(common_ptr, show_history);

    if (show_history) {
        showHistory(common_ptr);
    }
}

/**
 * @brief Thread operator to run the GUI main loop.
 * @param common Reference to the Common structure containing shared data.
 */
void DrawThread::operator()(Common& common) {
    GuiMain(DrawAppWindow, &common); // Run the GUI main loop with the DrawAppWindow function
    std::cout << "Gui Main Finished!!" << std::endl;
    common.exit_flag = true; // Set exit flag when the GUI main loop finishes
}

/**
 * @brief Displays the title and subtitle with a fade-in effect.
 * @param titleFont Pointer to the ImFont object for the title font.
 * @param subtitleFont Pointer to the ImFont object for the subtitle font.
 */
void title(ImFont* titleFont, ImFont* subtitleFont) {
    // Increment alpha values for fade-in effect
    title_alpha = min(title_alpha + alpha_increment, 1.0f);
    subtitle_alpha = min(subtitle_alpha + alpha_increment, 1.0f);

    // Set position and size of the title window
    ImGui::SetNextWindowPos(ImVec2(screenWidth - 1700, 120), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(380, 80), ImGuiCond_Always);

    // Title window styling and rendering
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.0f, 0.0f, title_alpha));

    ImGui::Begin("TitleWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PushFont(titleFont);
    ImGui::Text("SafetyNet");
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleColor(3);

    // Set position and size of the subtitle window
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(screenWidth) - 1700.0f, 200.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(700, 80), ImGuiCond_Always);

    // Subtitle window styling and rendering
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, subtitle_alpha));

    ImGui::Begin("SubtitleWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PushFont(subtitleFont);
    ImGui::Text("\tFinal project in the CPP course,\n\n\t\tPresented by Yarden Ben-Tal and Shlomi Edelstien.");
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleColor(3);
}

/**
 * @brief Displays the alert count in a dedicated window.
 * @param common_ptr Pointer to the Common structure containing shared data.
 * @param clockFont Pointer to the ImFont object for the clock font.
 */
void AlertCount(void* common_ptr, ImFont* clockFont) {
    auto common = (Common*)common_ptr;
    // Set position and size of the alert count window
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2) - 295, ImGui::GetIO().DisplaySize.y - 150), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(700, 80), ImGuiCond_Always);

    // Window styling and rendering
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ImGui::Begin("AlertsCounterWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PushFont(clockFont);
    if (common->data_ready) {
        ImGui::Text("Amount of Alerts in the last day: %d", common->todays_alerts.load());
    }
    else {
        ImGui::Text("Getting Data, Please stand by...");
    }
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleColor(3);
}

/**
 * @brief Displays the current time in a dedicated window.
 * @param clockFont Pointer to the ImFont object for the clock font.
 */
void Clock(ImFont* clockFont) {
    // Set position and size of the clock window
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2) - 100, ImGui::GetIO().DisplaySize.y - 100), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(200, 40), ImGuiCond_Always);

    // Window styling and rendering
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ImGui::Begin("ClockWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PushFont(clockFont);

    // Get current time and display
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    tm now_tm;
    localtime_s(&now_tm, &now_c);
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &now_tm);

    ImGui::Text("%s", buffer);
    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleColor(3);
}

/**
 * @brief Displays the live alerts window and handles alert display.
 * @param common_ptr Pointer to the Common structure containing shared data.
 */
void LiveAlertsWindow(void* common_ptr) {
    auto common = (Common*)common_ptr;
    static char live_buff[200] = "";
    static std::string live_active_search = "";
    static const char* default_search_text = "Insert the desired alerts location and press Enter:";

    // Search bar
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2) - 360, ImGui::GetIO().DisplaySize.y / 2 - 100), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(700, 40), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    ImGui::Begin("SearchBarWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (strlen(live_buff) == 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::InputTextWithHint("##search", default_search_text, live_buff, IM_ARRAYSIZE(live_buff));
        ImGui::PopStyleColor();
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        if (ImGui::InputTextWithHint("##search", default_search_text, live_buff, IM_ARRAYSIZE(live_buff), ImGuiInputTextFlags_EnterReturnsTrue)) {
            live_active_search = live_buff;
            live_buff[0] = '\0';
        }
        ImGui::PopStyleColor();
    }
    ImGui::PopStyleColor(3);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));

    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 70);
    if (ImGui::Button("Clear")) {
        live_buff[0] = '\0';
        live_active_search.clear();
    }

    ImGui::End();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    // Live alerts window
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2) - 360, ImGui::GetIO().DisplaySize.y / 2 - 40), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(700, 250), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.0f);

    ImGui::Begin("LiveAlertWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (!live_active_search.empty()) {
        ImGui::Text("Now showing alerts for: %s", live_active_search.c_str());
    }

    bool found_alerts = false;
    bool noAlertsForLocation = true;

    if (!common->no_live_alerts) {
        for (const auto& rec : common->LiveAlerts) {
            if (rec.data.find(live_active_search) != std::string::npos) {
                noAlertsForLocation = false;
                break;
            }
        }
        if (!noAlertsForLocation) {
            if (ImGui::BeginTable("Live Alerts Table", 4, ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("Date and time");
                ImGui::TableSetupColumn("Type");
                ImGui::TableSetupColumn("Location");
                ImGui::TableSetupColumn("Category");
                ImGui::TableHeadersRow();

                std::unique_lock<std::mutex> lock(common->Live_mtx);
                common->Live_cv.wait(lock, [&] { return common->data_ready.load(); });

                ImVec4 powerfulRed = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, powerfulRed);

                for (const auto& rec : common->LiveAlerts) {
                    if (rec.data.find(live_active_search) != std::string::npos) {
                        found_alerts = true;
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text(rec.alertDate.c_str());

                        ImGui::TableSetColumnIndex(1);
                        std::string reversed = reverse_utf8(rec.title);
                        ImGui::Text(reversed.c_str());

                        ImGui::TableSetColumnIndex(2);
                        reversed = reverse_utf8(rec.data);
                        ImGui::Text(reversed.c_str());

                        ImGui::TableSetColumnIndex(3);
                        ImGui::Text("%d", rec.category);
                    }
                }

                ImGui::PopStyleColor();
                common->Live_cv.notify_all();
                ImGui::EndTable();
            }
        }
    }

    if (!found_alerts || noAlertsForLocation) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::SetWindowFontScale(1.2f);
        ImGui::Text("No live alerts, thank god!");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();
    }

    ImGui::End();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
}

/**
 * @brief Displays the instructions window.
 * @param p_open Pointer to a boolean indicating whether the window should be open.
 */
void show_instructions(bool* p_open) {
    // Set position and size of the instructions window
    ImGui::SetNextWindowPos(ImVec2(120, 470), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Always);

    // Window styling and rendering
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ImGui::Begin("Instructions", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Instructions:");
    ImGui::Separator();
    ImGui::TextWrapped("1. View live alerts from all locations at the center.");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::TextWrapped("2. Use the search bar to enter your desired location.");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::TextWrapped("3. Press 'Enter' to update the live alerts for the specified location.");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::TextWrapped("4. Click 'Alert's History' to view historical alert data.");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::TextWrapped("5. Press 'Exit' or use ESC to close the application.");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::TextWrapped("6. The current time and the total number of alerts in the last day are displayed at the bottom.");
    ImGui::End();
    ImGui::PopStyleColor(3);
}

/**
 * @brief Displays the alert history window.
 * @param common_ptr Pointer to the Common structure containing shared data.
 */
void showHistory(void* common_ptr) {
    auto common = (Common*)common_ptr;
    static bool first_time = true;
    if (first_time) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 200, 70), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(1600, 350), ImGuiCond_Always);
        first_time = false;
    }

    ImGui::Begin("Alert's History", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
    ImGui::Text("Alert's History:");
    ImGui::Text("Search:");

    static char buff[200] = "";
    static std::string search_text;
    static std::string active_search = "";

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.0f, 0.0f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

    if (ImGui::InputText("##history_search", buff, sizeof(buff))) {
        search_text = buff;
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));

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

    ImGui::PopStyleColor(3);

    if (common->data_ready) {
        if (ImGui::BeginTable("Alerts History Table", 4)) {
            ImGui::TableSetupColumn("Date and time");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Location");
            ImGui::TableSetupColumn("Category");
            ImGui::TableHeadersRow();

            for (const auto& rec : common->Alerts) {
                if (rec.data.find(active_search) != std::string::npos) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text(rec.alertDate.c_str());
                    ImGui::TableSetColumnIndex(1);
                    std::string reversed = reverse_utf8(rec.title);
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
}

/**
 * @brief Displays the menu buttons for various actions.
 * @param common_ptr Pointer to the Common structure containing shared data.
 * @param show_history Reference to the boolean controlling the display of alert history.
 */
void menueButtons(void* common_ptr, bool& show_history) {
    auto common = (Common*)common_ptr;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    // "Exit" button
    ImGui::SetNextWindowPos(ImVec2(1700, 1000), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(200, 80), ImGuiCond_Always);
    ImGui::Begin("Exit Button", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (ImGui::Button("Exit", ImVec2(150, 50))) {
        common->exit_requested = true; // Set exit requested flag
    }
    ImGui::End();

    static bool show_instructions_window = false;

    // "Instructions" button
    ImGui::SetNextWindowPos(ImVec2(1700, 800), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(200, 80), ImGuiCond_Always);
    ImGui::Begin("Instructions Button", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (ImGui::Button("Instructions", ImVec2(150, 50))) {
        show_instructions_window = !show_instructions_window;
    }
    ImGui::End();
    if (show_instructions_window) {
        show_instructions(&show_instructions_window);
    }

    // "Alert's History" button
    ImGui::SetNextWindowPos(ImVec2(1700, 900), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(200, 80), ImGuiCond_Always);
    ImGui::Begin("Alert's History Button", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (ImGui::Button("Alert's History", ImVec2(150, 50))) {
        show_history = !show_history;
        if (!show_history) {
            title_alpha = 0.0f;
            subtitle_alpha = 0.0f;
        }
    }
    ImGui::End();

    ImGui::PopStyleColor(5);
}
