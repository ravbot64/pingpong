#ifndef PINGPONG_SIGNAL_H
#define PINGPONG_SIGNAL_H

// Lightweight async-signal-safe shutdown flag.
//
// install_shutdown_handler() catches SIGINT and SIGTERM. Hot loops should
// poll shutdown_requested() and exit cleanly so the final aggregate report
// is still printed.
void install_shutdown_handler();
bool shutdown_requested();

#endif
