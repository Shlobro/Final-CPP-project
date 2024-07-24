// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <string>
#include "GuiMain.h"
#include <thread>
#include <chrono>
using namespace std::chrono_literals;
#include <vector>
#include <ranges>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include "Common.h"

// show_instructions function!
void show_instructions(bool* p_open);
static bool show_instructions_window = false; 

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static ID3D11ShaderResourceView* g_backgroundTexture = nullptr;
static int g_backgroundWidth = 0;
static int g_backgroundHeight = 0;

// Simple helper function to load an image into a DX11 texture with common settings
bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    // Load from disk into a raw RGBA buffer
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
    pTexture->Release();

    *out_width = image_width;
    *out_height = image_height;
    stbi_image_free(image_data);

    return true;
}

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//RTL strings
// Function to determine the length of the UTF-8 character
size_t utf8_char_length(char c) {
    if ((c & 0x80) == 0x00) return 1; // 1-byte character
    if ((c & 0xE0) == 0xC0) return 2; // 2-byte character
    if ((c & 0xF0) == 0xE0) return 3; // 3-byte character
    if ((c & 0xF8) == 0xF0) return 4; // 4-byte character
    return 1; // Should not happen, treat as 1-byte character
}

// Function to reverse a UTF-8 string
std::string reverse_utf8(const std::string_view input) {
    std::vector<std::string_view> characters;
    size_t i = 0;

    while (i < input.size()) {
        size_t char_len = utf8_char_length(input[i]);
        characters.push_back(input.substr(i, char_len));
        i += char_len;
    }

    // Reverse the order of the characters and concatenate them using ranges and views
    auto reversed_view = characters | std::views::reverse | std::views::join;

    return std::string(reversed_view.begin(), reversed_view.end());
}

// Main code
int GuiMain(drawcallback drawfunction, void* obj_ptr)
{
    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);

    // Get screen resolution
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create a full screen window
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"IS_Alerts", WS_POPUP, 0, 0, screenWidth, screenHeight, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load the background texture
    if (!LoadTextureFromFile("black.jpg", &g_backgroundTexture, &g_backgroundWidth, &g_backgroundHeight))
    {
        std::cerr << "Failed to load background image!" << std::endl;
    }

    // Load Fonts
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0590, 0x05FF, // Greek and Coptic
        0,
    };
    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\calibrib.ttf", 18.0f, nullptr, &ranges[0]);
    IM_ASSERT(font != nullptr);
    ImFont* titleFont = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\calibriz.ttf", 70.0f, nullptr, &ranges[0]);
    IM_ASSERT(titleFont != nullptr);
    ImFont* subtitleFont = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\calibrib.ttf", 20.0f, nullptr, &ranges[0]);
    IM_ASSERT(subtitleFont != nullptr);
    ImFont* clockFont = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\calibrib.ttf", 36.0f, nullptr, &ranges[0]);
    IM_ASSERT(clockFont != nullptr);
    ImFont* defaultTextFont = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\calibrib.ttf", 25.0f, nullptr, &ranges[0]);
    IM_ASSERT(defaultTextFont != nullptr);
    ImFont* searchBarFont = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\calibrib.ttf", 17.0f, nullptr, &ranges[0]);
    IM_ASSERT(searchBarFont != nullptr);

    // Fade-in variables
    float title_alpha = 0.0f;
    float subtitle_alpha = 0.0f;
    const float alpha_increment = 0.009f; // Adjust the speed of the fade-in effect

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool exit_requested = false;
    bool show_alerts_history = false; // Add this flag

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Add a static "Exit" button
        ImGui::SetNextWindowPos(ImVec2(1700, 1000), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(200, 80), ImGuiCond_Always);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        ImGui::Begin("StaticButtonWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        if (ImGui::Button("Exit", ImVec2(150, 50)))
        {
            exit_requested = true;
        }
        ImGui::End();

        // Inside the main loop, before rendering ImGui:
ImGui::SetNextWindowPos(ImVec2(1700, 800), ImGuiCond_Always);
ImGui::SetNextWindowSize(ImVec2(200, 80), ImGuiCond_Always);

ImGui::Begin("InstructionsButtonWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
if (ImGui::Button("Instructions", ImVec2(150, 50)))
{
    show_instructions_window = !show_instructions_window;
}
ImGui::End();

if (show_instructions_window)
{
    show_instructions(&show_instructions_window);
}

        ImGui::SetNextWindowPos(ImVec2(1700, 900), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(200, 80), ImGuiCond_Always);

        ImGui::Begin("AlertHistoryButtonWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        if (ImGui::Button("Alert's History", ImVec2(150, 50)))
        {
            show_alerts_history = !show_alerts_history;
            if (!show_alerts_history)
            {
                title_alpha = 0.0f;
                subtitle_alpha = 0.0f;
            }
        }
        ImGui::End();

        ImGui::PopStyleColor(5); // Restore previous colors

        // Render the background image
        if (g_backgroundTexture)
        {
            ImGui::GetBackgroundDrawList()->AddImage(
                g_backgroundTexture,
                ImVec2(0, 0),
                ImVec2((float)g_backgroundWidth, (float)g_backgroundHeight)
            );
        }

        // Conditionally render the title and subtitle
        if (!show_alerts_history) {
            // Increment alpha values for fade-in effect
            if (title_alpha < 1.0f) title_alpha += alpha_increment;
            if (subtitle_alpha < 1.0f) subtitle_alpha += alpha_increment;

            // Static title "SafetyNet"
            ImGui::SetNextWindowPos(ImVec2(screenWidth - 1700, 120), ImGuiCond_Always); // place of the title
            ImGui::SetNextWindowSize(ImVec2(380, 80), ImGuiCond_Always); // Adjusted size of the title area

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));      // Window background color (transparent)
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));        // Border color (transparent)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.0f, 0.0f, title_alpha));   // Text color (red with alpha)

            ImGui::Begin("TitleWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            ImGui::PushFont(titleFont);
            ImGui::Text("SafetyNet");
            ImGui::PopFont();

            ImGui::End();

            ImGui::PopStyleColor(3); // Restore previous colors

            // Static subtitle
            ImGui::SetNextWindowPos(ImVec2(static_cast<float>(screenWidth) - 1700.0f, 200.0f), ImGuiCond_Always); // place of the subtitle (adjust the position as needed)
            ImGui::SetNextWindowSize(ImVec2(700, 80), ImGuiCond_Always); // Adjusted size of the subtitle area

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));      // Window background color (transparent)
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));        // Border color (transparent)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, subtitle_alpha)); // Text color (white with alpha)

            ImGui::Begin("SubtitleWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            ImGui::PushFont(subtitleFont);
            ImGui::Text("\tFinal project in the CPP course,\n\n\t\tPresented by Jordan 'Da Beast' and Shlomi 'Never here' Shwarts.");
            ImGui::PopFont();

            ImGui::End();

            ImGui::PopStyleColor(3); // Restore previous colors
        }

        // alerts count section
        ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2) - 295, ImGui::GetIO().DisplaySize.y - 150), ImGuiCond_Always); // Centered at the bottom
        ImGui::SetNextWindowSize(ImVec2(700, 80), ImGuiCond_Always); // Adjusted size of the subtitle area

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));      // Window background color (transparent)
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));        // Border color (transparent)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Text color (white with alpha)

        ImGui::Begin("AlertsCounterWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        // Retrieve the value from the atomic variable
        int todays_alerts = ((Common*)obj_ptr)->todays_alerts.load();

        // in the meantime its the same font as the clock
        ImGui::PushFont(clockFont);
        if (((Common*)obj_ptr)->data_ready) {
            ImGui::Text("Amount of Alerts in the last day: %d", todays_alerts);
        }
        else {
            ImGui::Text("Getting Data, Please stand by...");
        }
        ImGui::PopFont();

        ImGui::End();
        ImGui::PopStyleColor(3);

        // Clock Display
        ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2) - 100, ImGui::GetIO().DisplaySize.y - 100), ImGuiCond_Always); // Centered at the bottom
        ImGui::SetNextWindowSize(ImVec2(200, 40), ImGuiCond_Always); // Adjusted size of the clock area

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));      // Window background color (transparent)
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));        // Border color (transparent)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));          // Text color (white)

        ImGui::Begin("ClockWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImGui::PushFont(clockFont); // Use the larger font
        // Get current time
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        tm now_tm;
        localtime_s(&now_tm, &now_c);
        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &now_tm);

        ImGui::Text("%s", buffer);
        ImGui::PopFont(); // Restore the previous font

        ImGui::End();
        ImGui::PopStyleColor(3); // Restore previous colors

        // Add the static live alert window
        ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2) - 360, ImGui::GetIO().DisplaySize.y / 2 - 40), ImGuiCond_Always); // Centered in the screen
        ImGui::SetNextWindowSize(ImVec2(700, 250), ImGuiCond_Always); // Adjusted size of the alert window

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));      // Window background color (transparent)
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.0f, 0.0f, 1.0f));        // Border color (dark red)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));          // Text color (gray)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.0f);                     // Border thickness

        ImGui::Begin("LiveAlertWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImGui::PushFont(defaultTextFont); // Use the larger font for the live alert window

        const char* text = "Are You Ready For Some ROCKK&ROWWLLLL!?!!?";
        ImVec2 textSize = ImGui::CalcTextSize(text);
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 textPos = ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f);
        ImGui::SetCursorPos(textPos);

        ImGui::Text("%s", text);
        ImGui::PopFont(); // Restore the previous font

        ImGui::End();
        ImGui::PopStyleColor(3); // Restore previous colors
        ImGui::PopStyleVar();    // Restore previous style var

        // Add the search bar above the live alert window
        static char search_bar[256] = "";
        static const char* default_search_text = "Insert the desired fireworks location and press Enter:";
        ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x / 2) - 360, ImGui::GetIO().DisplaySize.y / 2 - 100), ImGuiCond_Always); // Centered above the live alert window
        ImGui::SetNextWindowSize(ImVec2(700, 40), ImGuiCond_Always); // Adjusted size of the search bar

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));      // Window background color (transparent)
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.0f, 0.0f, 1.0f));        // Border color (dark red)
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));       // Frame background color (transparent)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);                     // Border thickness

        ImGui::Begin("SearchBarWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImGui::PushFont(searchBarFont); // Use the search bar font

        if (strlen(search_bar) == 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));      // Default text color (gray)
            if (ImGui::InputTextWithHint("##search", default_search_text, search_bar, IM_ARRAYSIZE(search_bar), ImGuiInputTextFlags_EnterReturnsTrue)) {
                // Handle Enter key press event
                search_bar[0] = '\0'; // Clear the search bar text
            }
            ImGui::PopStyleColor();
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));      // User text color (white)
            if (ImGui::InputTextWithHint("##search", default_search_text, search_bar, IM_ARRAYSIZE(search_bar), ImGuiInputTextFlags_EnterReturnsTrue)) {
                // Handle Enter key press event
                search_bar[0] = '\0'; // Clear the search bar text
            }
            ImGui::PopStyleColor();
        }

        ImGui::PopFont(); // Restore the previous font

        ImGui::End();
        ImGui::PopStyleColor(3); // Restore previous colors
        ImGui::PopStyleVar();    // Restore previous style var


        // Call the user-defined draw function
        if (show_alerts_history)
        {
            drawfunction(obj_ptr);
        }

        // Check for ESC key press to exit the program
        if (io.KeysDown[VK_ESCAPE])
        {
            done = true;
        }

        // Show the demo window for testing
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Render ImGui
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0); // Present with vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

        // Check if exit was requested
        if (exit_requested)
            done = true;
    }

    // Cleanup
    if (g_backgroundTexture) { g_backgroundTexture->Release(); }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// instruction window
void show_instructions(bool* p_open)
{
    ImGui::SetNextWindowPos(ImVec2(120, 470), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));      // Window background color (transparent)
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));        // Border color (transparent)
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));          // Text color (white)

    ImGui::Begin("Instructions", p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Instructions:");
    ImGui::Separator();
    ImGui::TextWrapped("1. View live alerts from all locations at the center.");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));  // Add spacing
    ImGui::TextWrapped("2. Use the search bar to enter your desired location.");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));  // Add spacing
    ImGui::TextWrapped("3. Press 'Enter' to update the live alerts for the specified location.");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));  // Add spacing
    ImGui::TextWrapped("4. Click 'Alert's History' to view historical alert data.");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));  // Add spacing
    ImGui::TextWrapped("5. Press 'Exit' or use ESC to close the application.");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));  // Add spacing
    ImGui::TextWrapped("6. The current time and the total number of alerts in the last day are displayed at the bottom.");
    ImGui::End();

    ImGui::PopStyleColor(3); // Restore previous colors
}


// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
