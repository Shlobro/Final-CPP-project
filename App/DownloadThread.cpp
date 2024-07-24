#include "DownloadThread.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <chrono>
#include <thread>


void DownloadThread::operator()(Common& common)
{
    httplib::SSLClient cli("www.oref.org.il");

    // Set connection and read timeouts
    cli.set_connection_timeout(10); // 10 seconds
    cli.set_read_timeout(10);       // 10 seconds

    while (!common.exit_flag) {
        {
            std::unique_lock<std::mutex> lock(common.mtx);
            common.cv.wait(lock, [&] { return !common.data_ready; });

            std::cout << "Download thread running" << std::endl;

            auto res = cli.Get("/warningMessages/alert/History/AlertsHistory.json");

            int retry_count = 0;
            const int max_retries = 5;

            while (!res && retry_count < max_retries) {
                std::cerr << "Request failed, retrying... (" << retry_count + 1 << "/" << max_retries << ")" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                res = cli.Get("/warningMessages/alert/History/AlertsHistory.json");
                retry_count++;
            }

            if (res && res->status == 200) {
                std::cout << "Got the data, now writing to program..." << std::endl;
                common.todays_alerts = 0;

                try {
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
                    std::cerr << "JSON parsing error: " << e.what() << std::endl;
                }
            }
            else {
                if (!res) {
                    std::cerr << "Request failed after " << max_retries << " retries." << std::endl;
                }
                else {
                    std::cerr << "Request failed with status: " << res->status << std::endl;
                }
            }

            std::cout << "Finished downloading history" << std::endl;
            common.data_ready = true;
            common.cv.notify_all();
        }

        // Sleep outside the lock to allow other operations to proceed
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void DownloadThread::SetUrl(std::string_view new_url)
{
    _download_url = new_url;
}
