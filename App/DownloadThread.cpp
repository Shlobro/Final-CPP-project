#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "DownloadThread.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <thread>

/**
 * @brief Sets the history link URL.
 *
 * @param link The URL string to set as the history link.
 */
void DownloadThread::setHistoryLink(const std::string& link) {
    history_link = link;
}

/**
 * @brief Sets the live link URL.
 *
 * @param link The URL string to set as the live link.
 */
void DownloadThread::setLiveLink(const std::string& link) {
    live_link = link;
}

/**
 * @brief Fetches live alerts from the specified URL and updates the shared `Common` object.
 *
 * This function continuously fetches live alert data from the server. It parses the JSON
 * response and stores the data in the shared `Common` object. The function retries up to
 * a specified number of times in case of a failed request and notifies waiting threads
 * upon data availability.
 *
 * @param common Reference to the shared `Common` object for synchronization and data sharing.
 */
void DownloadThread::liveAlerts(Common& common) {
    httplib::SSLClient cli("www.oref.org.il");

    // Set connection and read timeouts (10 seconds each)
    cli.set_connection_timeout(std::chrono::seconds(10)); // to the server
    cli.set_read_timeout(std::chrono::seconds(10)); // from the server

    while (!common.exit_flag) {
        std::cout << "Live Download thread running" << std::endl; // Our wonderful debug line

        auto data = cli.Get(live_link);
        int retry_count = 0;
        const int max_retries = 5;

        // Retry loop for failed requests
        while (!data && retry_count < max_retries) {
            std::cerr << "Request failed, retrying... (" << retry_count + 1 << "/" << max_retries << ")" << std::endl; // Our wonderful debug line
            std::this_thread::sleep_for(std::chrono::seconds(1)); // wait 1 sec and then retries again
            data = cli.Get(live_link);
            retry_count++;
        }

        if (data && data->status == 200) { //success in downloading data
            std::cout << "Got the data, now writing to program..." << std::endl;
            // critical section using lock guard in this scope
            {std::lock_guard<std::mutex> lock(common.Live_mtx);
                // If there are no live alerts the Json will not be in the format we are trying to parse it in. 
                // It will throw in error we are expecting to indicate that there are no live alerts.

                try {
                    // Parse JSON response
                    auto json_result = nlohmann::json::parse(data->body);

                    // Lock the mutex for thread safety
                    common.LiveAlerts.clear(); // delets all the current data in vector

                    for (const auto& alert : json_result) { // Json parsing
                        Alert m;
                        m.alertDate = alert["alertDate"];
                        m.title = alert["title"];
                        m.data = alert["data"];
                        m.category = alert["category"];
                        common.LiveAlerts.push_back(m); // Insert to the vector
                    }
                    // Update no_live_alerts flag based on data availability
                    common.no_live_alerts = common.LiveAlerts.empty();
                }

                // The spesific error that tells us the Json is empty
                catch (const nlohmann::json::parse_error& e) {
                    // Handle JSON parsing errors
                    std::cerr << "JSON parsing error: " << e.what() << std::endl;
                    if (std::string(e.what()).find("parse error at line 2, column 1: syntax error while parsing value - unexpected end of input") != std::string::npos) {
                        common.no_live_alerts = true;
                        std::cout << "The live alerts JSON is empty." << std::endl; // Our wonderful debug line
                    }
                }

                // General error
                catch (const std::exception& e) {
                    // Handle other exceptions
                    std::cerr << "JSON parsing error: " << e.what() << std::endl;
                }
            }// end of scope of the lock_guard
        }

        else {
            // Handle request failures (for debug purpose)
            if (!data) {
                std::cerr << "Request failed after " << max_retries << " retries." << std::endl; // Our wonderful debug line
            }
            else {
                std::cerr << "Request failed with status: " << data->status << std::endl; // Our very wonderful debug line
            }
        }

        std::cout << "Finished downloading live alerts" << std::endl; // Our very very wonderful debug line

        // Wait for 5 seconds before fetching the next update
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

/**
 * @brief Main function for the download thread, fetching alert history.
 *
 * This function is the main entry point for the download thread. It fetches
 * historical alert data, parses it, and stores it in the shared `Common` object. The
 * function waits on a condition variable to synchronize with other threads.
 *
 * @param common Reference to the shared `Common` object for synchronization and data sharing.
 */
void DownloadThread::alertsHistory(Common& common) {
    httplib::SSLClient cli("www.oref.org.il");

    // Set connection and read timeouts (10 seconds each)
    cli.set_connection_timeout(std::chrono::seconds(10)); // to the server
    cli.set_read_timeout(std::chrono::seconds(10)); // from the server

    while (!common.exit_flag) {
        {

            // Fetch alert history data
            auto data = cli.Get(history_link);
            int retry_count = 0;
            const int max_retries = 5;

            // Retry loop for failed requests
            while (!data && retry_count < max_retries) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                data = cli.Get(history_link);
                retry_count++;
            }

            if (data && data->status == 200) {
                std::unique_lock<std::mutex> lock(common.history_mtx);
                //as long the history data is ready or the exit flag is false -> then wait
                common.history_cv.wait(lock, [&] { return !common.history_data_ready || common.exit_flag; });
                common.todays_alerts = 0; // Counter for today alerts showing on main screen
                common.history_alerts.clear(); // clear the vector
                try {
                    // Parse JSON response
                    auto json_result = nlohmann::json::parse(data->body);
                    for (const auto& alert : json_result) {
                        Alert m;
                        m.alertDate = alert["alertDate"];
                        m.title = alert["title"];
                        m.data = alert["data"];
                        m.category = alert["category"];
                        common.history_alerts.push_back(m);
                        common.todays_alerts++;
                    }
                }
                catch (const std::exception& e) {
                    // Handle JSON parsing specific errors like b4
                    std::cerr << "JSON parsing error: " << e.what() << std::endl;
                }
            }
            else {
                // Handle general request failures
                if (!data) {
                    std::cerr << "Request failed after " << max_retries << " retries." << std::endl; // Our wonderful debug line
                }
                else {
                    std::cerr << "Request failed with status: " << data->status << std::endl; // Our very wonderful debug line
                }
            }

            common.history_data_ready = true;
        }

        // Allow other operations to proceed
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
