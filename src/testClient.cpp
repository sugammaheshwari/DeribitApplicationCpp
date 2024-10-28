#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/memory.hpp>
#include <thread>

typedef websocketpp::client<websocketpp::config::asio> client;

class WebSocketClient
{
public:
    WebSocketClient()
    {
        // Initialize ASIO
        m_endpoint.init_asio();

        // Set handlers
        m_endpoint.set_open_handler(
            std::bind(&WebSocketClient::on_open, this, std::placeholders::_1));
        m_endpoint.set_message_handler(
            std::bind(&WebSocketClient::on_message, this, std::placeholders::_1, std::placeholders::_2));
        m_endpoint.set_close_handler(
            std::bind(&WebSocketClient::on_close, this, std::placeholders::_1));
    }

    void connect(const std::string &uri)
    {
        websocketpp::lib::error_code ec;
        client::connection_ptr con = m_endpoint.get_connection(uri, ec);

        if (ec)
        {
            std::cout << "Could not create connection because: " << ec.message() << std::endl;
            return;
        }

        m_handle = con->get_handle(); // Store the handle for later use
        m_endpoint.connect(con);

        // Start the ASIO io_service run in a separate thread
        std::thread([this]()
                    { m_endpoint.run(); })
            .detach();
    }

    void send_message(const std::string &message)
    {
        websocketpp::lib::error_code ec;
        m_endpoint.send(m_handle, message, websocketpp::frame::opcode::text, ec);
        if (ec)
        {
            std::cout << "Send failed: " << ec.message() << std::endl;
        }
    }

private:
    void on_open(websocketpp::connection_hdl hdl)
    {
        std::cout << "Connection opened." << std::endl;
        m_handle = hdl; // Update the handle on open

        // Send a message after opening the connection
        send_message("Hello, Server!");
    }

    void on_message(websocketpp::connection_hdl, client::message_ptr msg)
    {
        std::cout << "Received: " << msg->get_payload() << std::endl;
    }

    void on_close(websocketpp::connection_hdl)
    {
        std::cout << "Connection closed." << std::endl;
    }

    client m_endpoint;
    websocketpp::connection_hdl m_handle;
};

int main(int argc, char *argv[])
{
    // Check for the URI argument
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <ws://server-uri>" << std::endl;
        return 1;
    }

    std::string uri = argv[1];

    // Create and run the WebSocket client
    WebSocketClient client;
    client.connect(uri);

    // Keep the main thread alive to allow receiving messages
    std::this_thread::sleep_for(std::chrono::hours(1)); // Adjust as needed

    return 0;
}
