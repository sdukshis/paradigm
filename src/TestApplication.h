#ifndef TESTAPPLICATION_H
#define TESTAPPLICATION_H

#include "Application.h"
#include <thread>
#include <chrono>

class TestApplication: public Application {
 public:
    TestApplication()
        : step_counter(0)
    { }

    int acquireNonCriticalResources() override {
        step_counter = 0;
        return 0;
    }

    int step() override {
        ++step_counter;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return step_counter > 5;
    }

 private:
    int step_counter;
};

#endif