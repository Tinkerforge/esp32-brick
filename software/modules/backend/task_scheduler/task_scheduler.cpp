#include "task_scheduler.h"

extern AsyncWebServer server;

Task::Task(const char *task_name, std::function<void(void)> fn, uint32_t first_run_delay_ms, uint32_t delay_ms, bool once) :
          task_name(task_name),
          fn(std::move(fn)),
          next_deadline_ms(millis() + first_run_delay_ms),
          delay_ms(delay_ms),
          once(once) {

}

bool compare(const Task &a, const Task &b) {
    return a.next_deadline_ms >= b.next_deadline_ms;
}

void TaskScheduler::setup()
{
    initialized = true;
}

const char *current_scheduler_state = "init";
const char *current_scheduler_task = "init";

void TaskScheduler::register_urls()
{
    server.on("/scheduler_state", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", String(current_scheduler_state));
    });

    server.on("/scheduler_task", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", String(current_scheduler_task));
    });
}

void TaskScheduler::onEventConnect(AsyncEventSourceClient *client)
{

}

void TaskScheduler::loop() {
    current_scheduler_state = "checking for empty queue";
    if(tasks.empty()) {
        Serial.println("no tasks!");
        return;
    }
    current_scheduler_state = "top";
    auto &task_ref = tasks.top();
    current_scheduler_task = task_ref.task_name;
    if(!deadline_elapsed(task_ref.next_deadline_ms)) {
        current_scheduler_state = "not elapsed";
        return;
    }
    current_scheduler_state = "copying task";

    Task task = task_ref;
    tasks.pop();
    current_scheduler_state = "running task";

    task.fn();

    current_scheduler_state = "done running task";

    if(task.once) {
        current_scheduler_state = "task ran once";
        return;
    }
    current_scheduler_state = "pushing task";

    task.next_deadline_ms = millis() + task.delay_ms;
    tasks.push(std::move(task));

    current_scheduler_state = "end loop";
}


void TaskScheduler::scheduleOnce(const char *taskName, std::function<void(void)> &&fn, uint32_t delay){
    tasks.emplace(taskName, fn, delay, 0, true);
}

void TaskScheduler::scheduleWithFixedDelay(const char *taskName, std::function<void(void)> &&fn, uint32_t first_delay, uint32_t delay) {
    tasks.emplace(taskName, fn, first_delay, delay, false);
}

