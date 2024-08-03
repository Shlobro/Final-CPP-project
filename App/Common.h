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
    std::string title;     ///< The title of the alert.
    std::string data;      ///< The detailed data of the alert.
    int category;          ///< The category of the alert.
};

/**
 * @struct Common
 * @brief Shared resources and state management for alert handling.
 */
struct Common
{
    // General
    std::atomic_bool exit_flag = false; ///< Flag to signal exit.
    std::atomic_bool data_ready = false; ///< Flag indicating if data is ready.
    std::mutex mtx; ///< Mutex for general data protection.
    std::condition_variable cv; ///< Condition variable for general synchronization.
    std::string url; ///< URL for data retrieval.
    std::vector<Alert> Alerts; ///< Vector of all alerts.
    std::atomic_int todays_alerts = 0; ///< Counter for today's alerts.

    // Live alerts
    std::atomic_bool no_live_alerts = true; ///< Flag indicating the presence of live alerts.
    std::vector<Alert> LiveAlerts; ///< Vector of live alerts.
    std::mutex Live_mtx; ///< Mutex for live alerts data protection.
    std::condition_variable Live_cv; ///< Condition variable for live alerts synchronization.
    std::string Live_url; ///< URL for live data retrieval.
    std::atomic_int live_alerts = 0; ///< Counter for live alerts.
    std::atomic_bool live_data_ready = false; ///< Flag indicating if live data is ready.

    std::atomic_bool exit_requested = false; ///< Flag indicating if an exit has been requested.
};
