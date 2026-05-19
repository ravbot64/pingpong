# PingPong 🏓

A TCP bandwidth measurement tool. Measures network throughput between two machines (or localhost) by sending data as fast as possible over a TCP connection.

## What It Does

- Runs in **server** or **client** mode
- **Server** listens for a connection, receives data, and measures throughput
- **Client** connects to the server and blasts data for a configurable duration
- Both sides independently report bandwidth, transfer size, and elapsed time

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
| `-l <bytes>` | Read/write buffer size | 131072 (128 KB) |

### Examples

```bash
# Server on default port
./pingpong -s

# Server on custom port
./pingpong -s -p 9000

# Client: 10 second test to localhost
./pingpong -c 127.0.0.1

# Client: 30 second test
./pingpong -c 192.168.1.5 -t 30

# Client: custom port and buffer size
./pingpong -c 10.0.0.1 -p 9000 -l 65536
```

## Sample Output

**Server:**
```
PingPong v0.1
-------------------------------------------
Server listening on port 5201
-------------------------------------------
Accepted connection from 127.0.0.1:60462

Client disconnected.
- - - - - - - - - - - - - - - - - - -
[ ID]  Interval         Transfer      Bandwidth
[  0]  0.00-3.00 sec   45.39 GBytes   129.97 Gbits/sec
```

**Client:**
```
PingPong v0.1
Connecting to 127.0.0.1 port 5201
Connected.

- - - - - - - - - - - - - - - - - - -
[ ID]  Interval         Transfer      Bandwidth
[  0]  0.00-3.00 sec   45.39 GBytes   129.96 Gbits/sec
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
│   │   └── throughput.h / .cpp  # Byte accumulation & bandwidth calculation
│   ├── network/
│   │   └── socket.h / .cpp      # POSIX socket wrapper (RAII, move semantics)
│   ├── server/
│   │   └── tcp_server.h / .cpp  # Server: listen, accept, receive, report
│   └── client/
│       └── tcp_client.h / .cpp  # Client: connect, send, report
```

## Requirements
- g++ with C++17 support
