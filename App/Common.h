#pragma once
#include <atomic>
#include <string>
#include <vector>
#include <ctime>
#include <mutex>

struct Alert
{
    //TODO add all the stuff of an alert
    std::string alertDate;
    std::string title;
    std::string data;
    int category;
};

struct Common
{
    std::atomic_bool exit_flag = false;
    //std::atomic_bool start_download = false;
    std::atomic_bool data_ready = false;
    std::mutex mtx;
    std::condition_variable cv;


    std::string url;
    std::vector<Alert> Alerts; // a vector of all the alerts
    std::atomic_int todays_alerts = 0; // Counter for today's alerts
};
