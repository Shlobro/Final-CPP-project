#pragma once

#include "Common.h"

/**
 * @class DownloadThread
 * @brief Manages downloading tasks, including handling live alerts and alert history.
 */
class DownloadThread
{
public:
    /**
     * @brief Initiates the download process for historical alerts.
     *
     * This function operates as the main thread for downloading alert history data.
     * It fetches, parses, and stores historical alert data in the shared `Common` object.
     *
     * @param common A reference to the `Common` class instance containing shared resources.
     */
    void alertsHistory(Common& common);

    /**
     * @brief Handles the continuous download of live alerts.
     *
     * This function continuously fetches live alerts from the server, parses the
     * JSON responses, and updates the shared `Common` object with the latest data.
     *
     * @param common A reference to the `Common` class instance containing shared resources.
     */
    void liveAlerts(Common& common);

    /**
     * @brief Sets the URL link for downloading historical alert data.
     *
     * @param link The URL string to set as the history link.
     */
    void setHistoryLink(const std::string& link);

    /**
     * @brief Sets the URL link for downloading live alert data.
     *
     * @param link The URL string to set as the live link.
     */
    void setLiveLink(const std::string& link);

private:
    std::string history_link; ///< URL link for downloading historical alerts.
    std::string live_link;    ///< URL link for downloading live alerts.
};
