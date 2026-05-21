#include "signal.h"

#include <atomic>
#include <csignal>

namespace {

// sig_atomic_t is the only type guaranteed to be safe to write from a signal
// handler. std::atomic<bool> is *also* fine on every platform we care about
// (it's lock-free for bool), but sig_atomic_t keeps us aligned with POSIX.
volatile std::sig_atomic_t g_shutdown_flag = 0;

extern "C" void on_signal(int /*sig*/) {
    g_shutdown_flag = 1;
}

} // namespace

void install_shutdown_handler() {
    struct sigaction sa{};
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // intentionally NOT SA_RESTART so blocking syscalls return EINTR
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    // We use MSG_NOSIGNAL on send(), but be defensive: ignore SIGPIPE globally
    // so a peer close while writing never kills the process.
    struct sigaction ignore{};
    ignore.sa_handler = SIG_IGN;
    sigemptyset(&ignore.sa_mask);
    sigaction(SIGPIPE, &ignore, nullptr);
}

bool shutdown_requested() {
    return g_shutdown_flag != 0;
}
