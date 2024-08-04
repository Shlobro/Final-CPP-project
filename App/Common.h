#pragma once

#include <atomic>
#include <string>
#include <vector>
#include <ctime>
#include <mutex>

/**
 * @struct Alert
 * @brief Structure representing an alert with its details.
 */
struct Alert
{
    std::string alertDate; ///< The date of the alert.
    std::string title;     ///< The type of the alert.
    std::string data;      ///< The location of the alert.
    int category;          ///< The category of the alert; 1 for missiles, 2 for enemy aircraft
};

/**
 * @struct Common
 * @brief All the shared resources and the state management for threads.
 */
struct Common // "Common" to all the thread
{
    // General
    std::atomic_bool exit_flag = false; ///< Flag to signal exit so the download thread knows when to exit.
    std::atomic_bool exit_requested = false; ///< Flag indicating if an exit has been requested (in GuiMain).

    // Alerts history
    std::atomic_bool history_data_ready = false; ///< Flag indicating if the download thread data is ready.
    std::mutex history_mtx; ///< Mutex for general data protection.
    std::condition_variable history_cv; ///< Condition variable for general synchronization for Alerts_history.
    std::vector<Alert> history_alerts; ///< Vector of all alerts in Alerts_history Json format.
    std::atomic_int todays_alerts = 0; ///< Counter for today's alerts.

    // Live alerts
    std::atomic_bool no_live_alerts = true; ///< Flag indicating the presence of live alerts.
    std::vector<Alert> LiveAlerts; ///< Vector of live alerts.
    std::mutex Live_mtx; ///< Mutex for live alerts data protection.
   };
