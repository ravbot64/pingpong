#ifndef PINGPONG_TCP_CLIENT_H
#define PINGPONG_TCP_CLIENT_H

#include "../common/config.h"
#include "../network/socket.h"
#include "../metrics/throughput.h"

class TcpClient {
public:
    explicit TcpClient(const Config& config);
    void run();

private:
    Config config_;
    Socket socket_;
    Throughput throughput_;
};

#endif
