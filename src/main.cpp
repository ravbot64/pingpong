#include <iostream>

#include "common/config.h"
#include "common/signal.h"
#include "server/tcp_server.h"
#include "client/tcp_client.h"

int main(int argc, char* argv[]) {
    std::cout << "PingPong v0.2" << std::endl;

    Config config = parse_args(argc, argv);
    install_shutdown_handler();

    switch (config.mode) {
        case Mode::SERVER: {
            TcpServer server(config);
            server.run();
            break;
        }
        case Mode::CLIENT: {
            TcpClient client(config);
            client.run();
            break;
        }
    }

    return 0;
}
