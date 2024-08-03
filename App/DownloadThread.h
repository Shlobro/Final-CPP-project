#pragma once

#include "Common.h"

/**
 * @class DownloadThread
 * @brief Handles downloading tasks, including live alerts.
 */
class DownloadThread
{
public:
    /**
     * @brief Operator to start the download process.
     * @param common A reference to the Common class instance containing shared resources.
     */
    void operator()(Common& common);

    /**
     * @brief Function to handle live alerts.
     * @param common A reference to the Common class instance containing shared resources.
     */
    void liveAlerts(Common& common);

private:
    std::string _download_url; ///< The URL for downloading resources.
};
