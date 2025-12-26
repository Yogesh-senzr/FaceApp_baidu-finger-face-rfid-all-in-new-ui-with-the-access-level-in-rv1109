#ifndef INC_HELPER_TIMER_TIMER_H
#define INC_HELPER_TIMER_TIMER_H
#include <interface/bface_export.h>
#include <chrono>
#include <string>

namespace bface {
namespace helper {
// Create a scoped timer. Result will be printed to vlog
class BFACE_API ScopedMicrosecondTimer {
public:
    explicit ScopedMicrosecondTimer(const std::string &text_id, int vlog_level);

    ~ScopedMicrosecondTimer();

private:
    const std::string text_id_;
    const int vlog_level_;
    std::chrono::time_point<std::chrono::steady_clock> t_start_;
};

// Create a scoped timer.
// The construction time and the deconstruction timestamp will be logged
class ScopedLoopProfilingTimer {
public:
    explicit ScopedLoopProfilingTimer(const std::string &text_id, int vlog_level);

    ~ScopedLoopProfilingTimer();

private:
    const std::string text_id_;
    const int vlog_level_;
    std::chrono::time_point<std::chrono::steady_clock> t_start_;
};

// Create a timer. Result will be printed to vlog
class BFACE_API MicrosecondTimer {
public:
    explicit MicrosecondTimer(const std::string &text_id, int vlog_level);

    MicrosecondTimer();

    int end();

    ~MicrosecondTimer();

private:
    bool has_ended_;
    const std::string text_id_;
    const int vlog_level_;
    std::chrono::time_point<std::chrono::steady_clock> t_start_;
};
}  /// namespace helper
}  /// namespace bface
#endif  // INC_HELPER_TIMER_TIMER_H
