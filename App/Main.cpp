// ConnectedApp.cpp : This file contains the 'main' function. Program execution begins and ends here.

#include <iostream>
#include <thread>
#include "Common.h"
#include "DrawThread.h"
#include "DownloadThread.h"

int main() {
    // Create shared object for data communication between threads
    Common common;

    // Create and start the drawing thread
    DrawThread draw;
    auto draw_th = std::jthread([&] { draw(common); });
    
    // Create and start the download thread for historical data
    DownloadThread down;

    down.setHistoryLink("/warningMessages/alert/History/AlertsHistory.json");
    down.setLiveLink("/warningMessages/alert/Alerts.json");

    auto down_th = std::jthread([&] { down.alertsHistory(common); });

    // Create and start the download thread for live alerts
    auto ldown_th = std::jthread([&] { down.liveAlerts(common); });

    // Inform the user that the program is running (for debug purpose)
    std::cout << "Running...\n";

    // Wait for the threads to finish execution
    draw_th.join();
    down_th.join();
    ldown_th.join();
 
    return 0;
}
