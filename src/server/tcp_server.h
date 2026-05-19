#ifndef PINGPONG_TCP_SERVER_H
#define PINGPONG_TCP_SERVER_H

#include "../common/config.h"
#include "../network/socket.h"
#include "../metrics/throughput.h"

class TcpServer {
public:
    explicit TcpServer(const Config& config);
    void run();

private:
    Config config_;
    Socket listen_socket_;
    Throughput throughput_;
};

#endif
