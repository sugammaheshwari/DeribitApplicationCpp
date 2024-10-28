#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/client.hpp>

using json = nlohmann::json;
typedef websocketpp::client<websocketpp::config::asio> client;

// Client class to handle WebSocket connection and messages
class echo_server_client
{
public:
    echo_server_client()
    {
        m_client.init_asio();
        m_client.set_open_handler(websocketpp::lib::bind(&echo_server_client::on_open, this, websocketpp::lib::placeholders::_1));
        m_client.set_message_handler(websocketpp::lib::bind(&echo_server_client::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        m_client.set_close_handler(websocketpp::lib::bind(&echo_server_client::on_close, this, websocketpp::lib::placeholders::_1));
        m_client.set_fail_handler(websocketpp::lib::bind(&echo_server_client::on_fail, this, websocketpp::lib::placeholders::_1));
    }

    void connect(const std::string &uri)
    {
        std::cout << __FUNCTION__ << std::endl;

        websocketpp::lib::error_code ec;
        client::connection_ptr con = m_client.get_connection(uri, ec);
        if (ec)
        {
            std::cerr << "Could not create connection: " << ec.message() << std::endl;
            return;
        }

        m_client.connect(con);
        // Start the WebSocket client in a new thread
        std::thread wsThread([this]()
                             { m_client.run(); });
        wsThread.detach();
    }

    void subscribe(const std::string &symbols)
    {
        std::cout << __FUNCTION__ << std::endl;
        json sub_request = {{"subscribe", symbols}};
        std::cout << "Subscribing to symbols on orderBookServer :" << symbols << std::endl;
        m_client.send(m_hdl, sub_request.dump(), websocketpp::frame::opcode::text);
    }

private:
    void on_open(websocketpp::connection_hdl hdl)
    {
        m_hdl = hdl;
        std::cout << "Connected to order book server." << std::endl;
        // Send subscription request
        // Subscription symbols will be passed from main
    }

    void on_message(websocketpp::connection_hdl, client::message_ptr msg)
    {
        try
        {
            // Parse the string into a nlohmann::json object
            nlohmann::json json_msg = nlohmann::json::parse(msg->get_payload());
            std::cout << "\nMessage received  " << json_msg.dump(4) << std::endl; // Pretty print with 4 spaces indentation
        }
        catch (const nlohmann::json::parse_error &e)
        {
            // add error handling in client
        }
    }

    void on_close(websocketpp::connection_hdl)
    {
        std::cout << "Connection closed." << std::endl;
    }

    void on_fail(websocketpp::connection_hdl)
    {
        std::cerr << "Connection failed." << std::endl;
    }

    client m_client;
    websocketpp::connection_hdl m_hdl;
};

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <url> " << " <symbols>\n";
        std::cerr << "Example: " << argv[0] << " ws://localhost:9002 " << " BTC-PERPETUAL,ETH-PERPETUAL\n";
        return 1;
    }

    std::string uri = argv[1];     // Your order book server URI
    std::string symbols = argv[2]; // Get subscription symbols from command-line arguments

    echo_server_client client;
    client.connect(uri);

    std::this_thread::sleep_for(std::chrono::seconds(1)); // wait 1 sec before sending subscription request to echo_server

    // Subscribe to the symbols after connecting
    client.subscribe(symbols);

    // Keep the main thread alive while the WebSocket client runs in the background
    std::this_thread::sleep_for(std::chrono::hours(1)); // Run for 1 hour or until stopped

    return 0;
}
