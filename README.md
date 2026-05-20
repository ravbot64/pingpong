# PingPong 🏓

A TCP bandwidth measurement tool. Measures network throughput between two machines (or localhost) by sending data as fast as possible over a TCP connection.

## What It Does

- Runs in **server** or **client** mode
- **Client** connects to the server and blasts data for a configurable duration
- **Server** listens for connections, receives data, and measures throughput
- Both sides report **live per-interval** throughput + a final aggregate summary
- Server stays up between tests, handling one client at a time

## Build

```bash
make
```

This produces a `pingpong` binary in the project root.

To clean up:

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
./pingpong -c <server-ip>
```

### Options

| Flag | Description | Default |
|------|-------------|---------|
| `-s` | Run as server | — |
| `-c <host>` | Run as client, connect to host | — |
| `-p <port>` | Port to listen on / connect to | 5201 |
| `-t <seconds>` | Test duration (client only) | 10 |
| `-i <seconds>` | Reporting interval | 1 |
| `-l <bytes>` | Read/write buffer size | 131072 (128 KB) |

### Examples

```bash
# Server on default port
./pingpong -s

# Server on custom port
./pingpong -s -p 9000

# Client: 10 second test to localhost
./pingpong -c 127.0.0.1

# Client: 30 second test with 2-second reporting intervals
./pingpong -c 192.168.1.5 -t 30 -i 2

# Client: custom port and buffer size
./pingpong -c 10.0.0.1 -p 9000 -l 65536
```

## Sample Output

**Client:**
```
PingPong v0.1
Connecting to 127.0.0.1 port 5201
Connected.
[ ID]  Interval         Transfer      Bandwidth
[  0]   0.00-1.00  sec   14.93 GBytes   128.26 Gbits/sec
[  0]   1.00-2.00  sec   15.41 GBytes   132.41 Gbits/sec
[  0]   2.00-3.00  sec   15.47 GBytes   132.91 Gbits/sec
[  0]   3.00-4.00  sec   15.45 GBytes   132.72 Gbits/sec
[  0]   4.00-5.00  sec   15.41 GBytes   132.38 Gbits/sec
- - - - - - - - - - - - - - - - - - -
[  0]   0.00-5.00  sec   76.68 GBytes   131.73 Gbits/sec
```

**Server:**
```
PingPong v0.1
-------------------------------------------
Server listening on port 5201
-------------------------------------------
Accepted connection from 127.0.0.1:49351
[ ID]  Interval         Transfer      Bandwidth
[  0]   0.00-1.00  sec   14.93 GBytes   128.26 Gbits/sec
[  0]   1.00-2.00  sec   15.41 GBytes   132.41 Gbits/sec
[  0]   2.00-3.00  sec   15.47 GBytes   132.91 Gbits/sec
[  0]   3.00-4.00  sec   15.45 GBytes   132.72 Gbits/sec
[  0]   4.00-5.00  sec   15.41 GBytes   132.38 Gbits/sec
- - - - - - - - - - - - - - - - - - -
[  0]   0.00-5.00  sec   76.68 GBytes   131.73 Gbits/sec

Client disconnected.
-------------------------------------------
Server listening on port 5201
-------------------------------------------
```

## Project Structure

```
pingpong/
├── Makefile
├── src/
│   ├── main.cpp                 # Entry point, arg parsing dispatch
│   ├── common/
│   │   ├── config.h / .cpp      # CLI argument parsing & configuration
│   │   └── units.h / .cpp       # Human-readable formatting (Mbits/sec, GBytes)
│   ├── metrics/
│   │   ├── timer.h / .cpp       # CLOCK_MONOTONIC timing utility
│   │   └── throughput.h / .cpp  # Byte accumulation, interval tracking & reporting
│   ├── network/
│   │   └── socket.h / .cpp      # POSIX socket wrapper (RAII, move semantics)
│   ├── server/
│   │   └── tcp_server.h / .cpp  # Server: listen, accept loop, receive, report
│   └── client/
│       └── tcp_client.h / .cpp  # Client: connect, send, report
```

## Requirements
- g++ with C++17 support

