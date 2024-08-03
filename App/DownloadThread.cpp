#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "DownloadThread.h"

#include "httplib.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <chrono>
#include <thread>

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
    cli.set_connection_timeout(std::chrono::seconds(10));
    cli.set_read_timeout(std::chrono::seconds(10));


    while (!common.exit_flag) {
        std::cout << "Live Download thread running" << std::endl;

        std::string link = "/warningMessages/alert/Alerts.json";
        //For historical data: 
        //std::string link = "/warningMessages/alert/History/AlertsHistory.json";

        auto res = cli.Get(link);
        int retry_count = 0;
        const int max_retries = 5;

        // Retry loop for failed requests
        while (!res && retry_count < max_retries) {
            std::cerr << "Request failed, retrying... (" << retry_count + 1 << "/" << max_retries << ")" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            res = cli.Get(link);
            retry_count++;
        }

        if (res && res->status == 200) {
            std::cout << "Got the data, now writing to program..." << std::endl;
            common.live_alerts = 0;

            try {
                // Parse JSON response
                auto json_result = nlohmann::json::parse(res->body);

                // Lock the mutex for thread safety
                std::unique_lock<std::mutex> lock(common.Live_mtx);
                common.data_ready = false;
                common.LiveAlerts.clear();

                for (const auto& alert : json_result) {
                    Alert m;
                    m.alertDate = alert["alertDate"];
                    m.title = alert["title"];
                    m.data = alert["data"];
                    m.category = alert["category"];
                    common.LiveAlerts.push_back(m);
                    common.live_alerts++;
                }

                // Update no_live_alerts flag based on data availability
                common.no_live_alerts = common.LiveAlerts.empty();
                common.data_ready = true;
                common.Live_cv.notify_all();

            }
            catch (const nlohmann::json::parse_error& e) {
                // Handle JSON parsing errors
                std::cerr << "JSON parsing error: " << e.what() << std::endl;
                if (std::string(e.what()).find("parse error at line 2, column 1: syntax error while parsing value - unexpected end of input") != std::string::npos) {
                    common.no_live_alerts = true;
                    common.data_ready = true;
                    std::cout << "The live alerts JSON is empty." << std::endl;
                }
                common.Live_cv.notify_all();
            }
            catch (const std::exception& e) {
                // Handle other exceptions
                std::cerr << "JSON parsing error: " << e.what() << std::endl;
                common.Live_cv.notify_all();
            }
        }
        else {
            // Handle request failures
            if (!res) {
                std::cerr << "Request failed after " << max_retries << " retries." << std::endl;
            }
            else {
                std::cerr << "Request failed with status: " << res->status << std::endl;
            }
        }

        std::cout << "Finished downloading live alerts" << std::endl;

        common.live_data_ready = true;
        common.Live_cv.notify_all();
        // Wait for 5 seconds before fetching the next update
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

/**
 * @brief Main function for the download thread, fetching alert history.
 *
 * This operator function is the main entry point for the download thread. It fetches
 * historical alert data, parses it, and stores it in the shared `Common` object. The
 * function waits on a condition variable to synchronize with other threads.
 *
 * @param common Reference to the shared `Common` object for synchronization and data sharing.
 */
void DownloadThread::operator()(Common& common) {
    httplib::SSLClient cli("www.oref.org.il");

    // Set connection and read timeouts (10 seconds each)
    cli.set_connection_timeout(std::chrono::seconds(10));
    cli.set_read_timeout(std::chrono::seconds(10));

    while (!common.exit_flag) {
        {
            std::unique_lock<std::mutex> lock(common.mtx);
            common.cv.wait(lock, [&] { return !common.data_ready || common.exit_flag; }); // <-- this line was changed to check for exit flag


            // Fetch alert history data
            auto res = cli.Get("/warningMessages/alert/History/AlertsHistory.json");

            int retry_count = 0;
            const int max_retries = 5;

            // Retry loop for failed requests
            while (!res && retry_count < max_retries) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                res = cli.Get("/warningMessages/alert/History/AlertsHistory.json");
                retry_count++;
            }

            if (res && res->status == 200) {
                common.todays_alerts = 0;

                try {
                    // Parse JSON response
                    auto json_result = nlohmann::json::parse(res->body);
                    for (const auto& alert : json_result) {
                        Alert m;
                        m.alertDate = alert["alertDate"];
                        m.title = alert["title"];
                        m.data = alert["data"];
                        m.category = alert["category"];
                        common.Alerts.push_back(m);
                        common.todays_alerts++;
                    }

                    if (!common.Alerts.empty()) {
                        common.data_ready = true;
                    }
                }
                catch (const std::exception& e) {
                    // Handle JSON parsing errors
                    std::cerr << "JSON parsing error: " << e.what() << std::endl;
                }
            }
            else {
                // Handle request failures
                if (!res) {
                    std::cerr << "Request failed after " << max_retries << " retries." << std::endl;
                }
                else {
                    std::cerr << "Request failed with status: " << res->status << std::endl;
                }
            }

            common.data_ready = true;
            common.cv.notify_all();
        }

        // Allow other operations to proceed
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
