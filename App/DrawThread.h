#pragma once

// Standard library includes
#include <string>
#include <iostream>

// Windows and DirectX includes
#include <d3d11.h>
#include <windows.h>

// ImGui includes
#include "imgui.h"

// Project-specific includes
#include "Common.h"
#include "GuiMain.h"

/**
 * @class DrawThread
 * @brief Class for handling drawing operations in a separate thread.
 */
class DrawThread
{
public:
    /**
     * @brief Function for executing drawing operations.
     * @param common A reference to the Common class instance containing shared resources.
     */
    void operator()(Common& common);
};

// Function declarations

/**
 * @brief Displays instruction window.
 * @param p_open Pointer to a boolean that controls the window's open state.
 */
void show_instructions();

/**
 * @brief Displays history information.
 * @param common_ptr Pointer to a Common class instance containing shared resources.
 */
void showHistory(void* common_ptr);

/**
 * @brief Displays menu buttons and handles their actions ("exit", "show history", "instructions").
 * @param common_ptr Pointer to a Common class instance containing shared resources.
 * @param show_history Reference to a boolean that indicates if history should be shown.
 */
void menuButtons(void* common_ptr, bool& show_history);

/**
 * @brief Displays the alert count.
 * @param common_ptr Pointer to a Common class instance containing shared resources.
 * @param clockFont Pointer to the ImFont used for displaying the clock.
 */
void AlertCount(void* common_ptr, ImFont* clockFont);

/**
 * @brief Displays a clock.
 * @param clockFont Pointer to the ImFont used for displaying the clock.
 */
void Clock(ImFont* clockFont);

/**
 * @brief Displays the title and subtitle.
 * @param titleFont Pointer to the ImFont used for displaying the title.
 * @param subtitleFont Pointer to the ImFont used for displaying the subtitle.
 */
void title(ImFont* titleFont, ImFont* subtitleFont);

/**
 * @brief Displays live alerts window.
 * @param common_ptr Pointer to a Common class instance containing shared resources.
 */
void LiveAlertsWindow(void* common_ptr);

/**
 * @brief The function we pass to GuiMain.
 * @param common_ptr Pointer to a Common class instance containing shared resources.
 */
void DrawAppWindow(void* common_ptr);