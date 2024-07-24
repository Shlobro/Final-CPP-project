// ConnectedApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include "Common.h"
#include "DrawThread.h"
#include "DownloadThread.h"



int main()
{
    Common common; // make an object that we defined
    DrawThread draw;    // make a thread instance that we defined
    auto draw_th = std::jthread([&] {draw(common); });  // create a new thread
    DownloadThread down;       // instance of a download thread object type
    auto down_th = std::jthread([&] {down(common); });  // activate the thread
    down.SetUrl("http://....");     // 
    std::cout << "running...\n";
    down_th.join();
    draw_th.join();
}

