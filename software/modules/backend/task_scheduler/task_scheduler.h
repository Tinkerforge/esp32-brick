#pragma once

#include "Arduino.h"

#include <vector>
#include <queue>
#include <functional>

#include <time.h>
#include <iostream>

#include "tools.h"

#include "ArduinoJson.h"
#include "AsyncJson.h"

struct Task {
    const char *task_name;
    std::function<void(void)> fn;
    uint32_t next_deadline_ms;
    uint32_t delay_ms;
    bool once;

    Task(const char *task_name, std::function<void(void)> fn, uint32_t first_run_delay_ms, uint32_t delay_ms, bool once);
};

bool compare(const Task &a, const Task &b);

class TaskScheduler {
public:
    TaskScheduler() : tasks(&compare) {}
    void setup();
    void register_urls();
    void onEventConnect(AsyncEventSourceClient *client);
    void loop();

    void scheduleOnce(const char *task_name, std::function<void(void)> &&fn, uint32_t delay) ;
    void scheduleWithFixedDelay(const char *task_name, std::function<void(void)> &&fn, uint32_t first_delay, uint32_t delay);
private:
    std::priority_queue<Task, std::vector<Task>, decltype(&compare)> tasks;
};
