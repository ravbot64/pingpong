# PingPong 🏓

A TCP bandwidth measurement tool. Measures network throughput between two machines (or localhost) by sending data as fast as possible over a TCP connection.

## What It Does

- Runs in **server** or **client** mode
- **Client** connects to the server and blasts data for a configurable duration
- **Server** listens for connections, receives data, and measures throughput
- Both sides report **fixed-grid per-interval** throughput + a final aggregate summary
- Server stays up between tests, handling one client at a time
- Resolves hostnames and supports both **IPv4 and IPv6**

## Build

```bash
make
```

This produces a `pingpong` binary in the project root. Builds are incremental — only changed sources recompile.

To clean:

```bash
make clean
```

## Usage

### Start the server

```bash
./pingpong -s
```

### Run a client test

```bash
./pingpong -c <host>
```

`<host>` may be a hostname (`server.local`), IPv4 literal (`192.168.1.5`), or IPv6 literal (`::1`).

### Options

| Flag | Description | Default |
|------|-------------|---------|
| `-s` | Run as server | — |
| `-c <host>` | Run as client, connect to host | — |
| `-B <bind_addr>` | Server bind address (use to restrict to one interface) | all interfaces |
| `-p <port>` | Port to listen on / connect to (1..65535) | 5201 |
| `-t <seconds>` | Test duration in seconds, client only (1..86400) | 10 |
| `-i <seconds>` | Reporting interval in seconds (0.01..60) | 1 |
| `-l <bytes>` | Read/write buffer size (1..16777216, i.e. 16 MiB) | 131072 (128 KiB) |
| `-T <seconds>` | Per-socket I/O timeout, 0 disables | 120 |
| `-h` | Show help | — |

`Ctrl-C` (`SIGINT`) on either side triggers a clean shutdown and still prints the aggregate report.

### Examples

```bash
# Server on default port, all interfaces
./pingpong -s

# Server on custom port, bound to localhost only
./pingpong -s -B 127.0.0.1 -p 9000

# Client: 10-second test to localhost (works with hostnames now)
./pingpong -c localhost

# Client: 30-second test with 2-second reporting intervals
./pingpong -c 192.168.1.5 -t 30 -i 2

# Client: IPv6
./pingpong -c ::1
```

## Measurement notes

- Bandwidth units use **SI** (`Mbits/sec`, `Gbits/sec` = ÷10⁶ / 10⁹).
- Transfer units use **binary** (`MBytes` = MiB, `GBytes` = GiB = ÷2²⁰ / 2³⁰).
- The **server's `t=0`** is anchored to the **first received byte**, not the moment it called `accept()`. This keeps connection-setup idle time out of the measurement, so the server's reported duration matches the client's.
- Intervals are emitted on a **fixed grid** (`[0,1)`, `[1,2)`, …) so boundaries don't drift over long tests. Each bucket's bandwidth is computed against the *configured* interval width, so back-to-back rows are comparable.
- The client reports bytes **accepted into the kernel send buffer**. The server side is the authoritative figure for actual on-wire throughput.

## Security note

The default bind address is `0.0.0.0` (all interfaces). There is **no authentication or encryption** — do not expose `pingpong` directly on an untrusted network. For host-local tests pass `-B 127.0.0.1`.

## Project structure

```
pingpong/
├── Makefile
├── src/
│   ├── main.cpp                 # Entry point, arg parsing dispatch
│   ├── common/
│   │   ├── config.{h,cpp}       # CLI parsing & validation
│   │   ├── signal.{h,cpp}       # SIGINT/SIGTERM shutdown flag
│   │   └── units.{h,cpp}        # Human-readable formatting
│   ├── metrics/
│   │   ├── timer.{h,cpp}        # CLOCK_MONOTONIC timing
│   │   └── throughput.{h,cpp}   # Byte counters, fixed-grid intervals, reporting
│   ├── network/
│   │   └── socket.{h,cpp}       # POSIX socket wrapper (RAII, IPv4/IPv6, timeouts)
│   ├── server/
│   │   └── tcp_server.{h,cpp}   # Server: listen, accept, receive, report
│   └── client/
│       └── tcp_client.{h,cpp}   # Client: resolve, connect, send, report
```

## Requirements

- g++ with C++17 support, POSIX (Linux/macOS)
