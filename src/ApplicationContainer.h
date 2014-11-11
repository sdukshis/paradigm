#ifndef APPLICATION_H
#define APPLICATION_H

#include <vector>
#include <memory>
#include <stdexcept>

#include "Config.h"
#include "Logging.h"
#include "Statistics.h"
#include "CmdListener.h"

class Application {
 public:
    Application(int argc, char *argv[]);

    Application(const Application &) = delete;

    void operator=(const Application &) = delete;

    int run();

 private:

    enum class State {
        INIT,
        ACQUIRE_CRITICAL,
        ACQUIRE_NONCRITICAL,
        STEP,
        RELEASE_NONCRITICAL,
        RELEASE_CRITICAL,
        FINISH
    };

    static const char * to_string(State);

    enum class Result {
        OK,
        STOP,
        FAIL
    };

    static const char * to_string(Result);

    // 
    //  Try/catch decorator.
    //  
    //  Translates any exception to Result::FAIL return code
    //
    template<Result (Application::*Func)()>
    Result tryTo();

    Result acquireCriticalResources();
    Result acquireNonCriticalResources();
    Result infrastructureStep();
    Result releaseNonCriticalResources();
    Result releaseCriticalResources();

    void registerSignalHandlers();

    static void signalHandler(int signum);

    bool isStopRequested() const { return !running_; }

    static std::atomic<bool> running_;
    State state_;
    Config conf_;
    Logger logger_;
    Statistics stats_;
    std::vector<std::unique_ptr<Application>> applications_;
};

std::atomic<bool> Application::running_(false);

Application::Application(int argc, char *argv[])
    : state_(State::INIT)
{
    conf_.parse(argc, argv);
}

int Application::run()
{
    running_ = true;
    Result rv;
    LOG(logger_, Logger::INFO, "Starting main loop");
    while (state_ != State::FINISH) {
        switch (state_) {
            case State::INIT:
                state_ = State::ACQUIRE_CRITICAL;
                break;
            case State::ACQUIRE_CRITICAL:
                switch (rv = tryTo<&Application::acquireCriticalResources>()) {
                    case Result::OK:
                        state_ = State::ACQUIRE_NONCRITICAL;
                        break;
                    case Result::FAIL:
                    case Result::STOP:
                        state_ = State::FINISH;
                        break;
                }
                break;
            case State::ACQUIRE_NONCRITICAL:
                switch (rv = tryTo<&Application::acquireNonCriticalResources>()) {
                    case Result::OK:
                        state_ = State::STEP;
                        break;
                    case Result::FAIL:
                        state_ = State::ACQUIRE_NONCRITICAL;
                        break;
                    case Result::STOP:
                        state_ = State::RELEASE_CRITICAL;
                        break;
                }
                break;
            case State::STEP:
                switch (rv = tryTo<&Application::infrastructureStep>()) {
                    case Result::OK:
                        state_ = State::STEP;
                        break;
                    case Result::STOP:
                    case Result::FAIL:
                        state_ = State::RELEASE_NONCRITICAL;
                        break;
                }
                break;
            case State::RELEASE_NONCRITICAL:
                switch (rv = tryTo<&Application::releaseNonCriticalResources>()) {
                    case Result::OK:
                    case Result::FAIL:
                        state_ = State::ACQUIRE_NONCRITICAL;
                        break;
                    case Result::STOP:
                        state_ = State::RELEASE_CRITICAL;
                        break;
                }
                break;
            case State::RELEASE_CRITICAL:
                switch (rv = tryTo<&Application::releaseCriticalResources>()) {
                    case Result::OK:
                    case Result::FAIL:
                    case Result::STOP:
                        state_ = State::FINISH;
                        break;
                }
                break;
            case State::FINISH:
                break;
        }
        LOG(logger_, Logger::DEBUG, "State: " << to_string(state_));
    }

    LOG(logger_, Logger::INFO, "Finish with retcode: " << (rv == Result::FAIL));

    return rv == Result::FAIL;
}

const char * 
Application::to_string(State state)
{
    switch (state) {
        case State::INIT:                return "INIT"; break;
        case State::ACQUIRE_CRITICAL:    return "ACQUIRE_CRITICAL"; break;
        case State::ACQUIRE_NONCRITICAL: return "ACQUIRE_NONCRITICAL"; break;
        case State::STEP:                return "STEP"; break;
        case State::RELEASE_NONCRITICAL: return "RELEASE_NONCRITICAL"; break;
        case State::RELEASE_CRITICAL:    return "RELEASE_CRITICAL"; break;
        case State::FINISH:              return "FINISH"; break;
    }
}

const char * 
Application::to_string(Result result)
{
    switch (result) {
        case Result::OK:   return "OK"; break;
        case Result::FAIL: return "FAIL"; break;
        case Result::STOP: return "STOP"; break;
    }
}


template<Application::Result (Application::*Func)()>
Application::Result
Application::tryTo() {
    Result rv;
    try {
        rv = (this->*Func)();
    } catch (const std::exception &e) {
        // TODO(pfilonov): log error
        rv = Result::FAIL;
    } catch (...) {
        // TODO(pfilonov): log unknown error message
        rv = Result::FAIL;
    }
    if (isStopRequested()) {
        LOG(logger_, Logger::INFO, "Stop requested");
        rv = Result::STOP;
    }
    LOG(logger_, Logger::DEBUG, "Result: " << to_string(rv));
    return rv;
}

Application::Result
Application::acquireCriticalResources()
{
    registerSignalHandlers();
    for (auto &app : applications_) {
        if (app->acquireCriticalResources()) {
            return Result::FAIL;
        }
    }
    return Result::OK;
}

Application::Result
Application::acquireNonCriticalResources()
{
    for (auto &app : applications_) {
        if (app->acquireNonCriticalResources()) {
            return Result::FAIL;
        }
    }
    return Result::OK;
}

Application::Result
Application::infrastructureStep()
{
    for (auto &app : applications_) {
        if (app->step()) {
            return Result::FAIL;
        }
    }
    return Result::OK;
}

Application::Result
Application::releaseNonCriticalResources()
{
    for (auto &app : applications_) {
        if (app->releaseNonCriticalResources()) {
            return Result::FAIL;
        }
    }
    return Result::OK;
}

Application::Result
Application::releaseCriticalResources()
{
    for (auto &app : applications_) {
        if (app->releaseCriticalResources()) {
            return Result::FAIL;
        }
    }
    return Result::OK;
}

void Application::registerSignalHandlers()
{
    if (SIG_ERR == signal(SIGINT, signalHandler)) {
        throw std::runtime_error("error registring signal hadnler");
    }
}

void Application::signalHandler(int signum)
{
    running_ = false;
}

#endif