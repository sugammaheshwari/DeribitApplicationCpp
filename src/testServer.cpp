#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using namespace websocketpp;

typedef server<config::asio> server_type; // Rename to avoid ambiguity

void on_open(connection_hdl hdl)
{
    std::cout << "New client connected!" << std::endl;
}

int main()
{
    server_type echo_server; // Use the renamed type

    echo_server.set_open_handler(&on_open);
    echo_server.init_asio();
    echo_server.set_reuse_addr(true);
    echo_server.listen(9004); // Change port if needed
    echo_server.start_accept();

    std::cout << "Echo server listening on port 9004..." << std::endl;
    echo_server.run();

    return 0;
}
