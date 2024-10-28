#include <iostream>
#include <string>
#include <deque>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <fstream>
#include <chrono>
#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <unordered_map>

using json = nlohmann::json;
typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::server<websocketpp::config::asio> echoServer;

// Data structures
std::unordered_map<std::string, std::string> order_books;             // Store order book data per symbol
std::unordered_map<std::string, std::shared_mutex> order_books_locks; // shared mutex for faster concurrent reads and single threaded writes.

std::unordered_map<std::string, std::vector<websocketpp::connection_hdl>> subscribed_clients; // Store which symbols are subscribed by which clients
std::mutex subscribed_clients_lock;                                                           // Lock for clientSocket symbol subscription list.

std::deque<std::string> broadcast_queue; // Queue to hold order book broadcast updates
std::mutex broadcast_queue_lock;         // Lock for broadcast queue

int msg_id = 0; // Global counter for message ID

class deribit_client
{
public:
    deribit_client()
        : uri("wss://test.deribit.com/ws/api/v2"),
          clientId("9YgyTMFg"),
          clientSecret("9xYau6uFd8PWR6iHvQNCNM4mFWl2Pr84wu20lS6NePc")
    {
        m_client.init_asio();

        m_client.set_tls_init_handler([this](websocketpp::connection_hdl hdl)
                                      { return this->on_tls_init(hdl); });

        m_client.set_open_handler(websocketpp::lib::bind(&deribit_client::on_open, this, websocketpp::lib::placeholders::_1));
        m_client.set_message_handler(websocketpp::lib::bind(&deribit_client::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        m_client.set_close_handler(websocketpp::lib::bind(&deribit_client::on_close, this, websocketpp::lib::placeholders::_1));
        m_client.set_fail_handler(websocketpp::lib::bind(&deribit_client::on_fail, this, websocketpp::lib::placeholders::_1));
    }

    void connect()
    {
        websocketpp::lib::error_code ec;
        client::connection_ptr con = m_client.get_connection(uri, ec);

        if (ec)
        {
            std::cerr << "Could not create connection: " << ec.message() << std::endl;
            return;
        }

        m_hdl = con->get_handle();
        m_client.connect(con);

        // Start a separate thread to handle WebSocket connection and events
        std::thread wsThread([this]()
                             { m_client.run(); });

        wsThread.detach(); // Detach the thread to run in the background
    }

    void send_authentication()
    {
        json auth_request = {
            {"jsonrpc", "2.0"},
            {"id", 9929},
            {"method", "public/auth"},
            {"params", {{"grant_type", "client_credentials"}, {"client_id", clientId}, {"client_secret", clientSecret}}}};

        this->send_request(auth_request);
        std::cout << "Authentication request sent: " << auth_request.dump() << std::endl;
    }

    void send_request(const json &request_msg)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_client.send(m_hdl, request_msg.dump(), websocketpp::frame::opcode::text);
    }

private:
    std::shared_ptr<boost::asio::ssl::context> on_tls_init(websocketpp::connection_hdl)
    {
        auto ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
        try
        {
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::no_sslv3 |
                             boost::asio::ssl::context::single_dh_use);
        }
        catch (std::exception &e)
        {
            std::cerr << "Error in TLS initialization: " << e.what() << std::endl;
        }
        return ctx;
    }

    void on_open(websocketpp::connection_hdl hdl)
    {
        std::cout << "Connection opened." << std::endl;
        send_authentication(); // Send authentication once connection is opened
    }

    void on_message(websocketpp::connection_hdl, client::message_ptr msg)
    {
        try
        {
            // Parse the string into a nlohmann::json object
            nlohmann::json json_msg = nlohmann::json::parse(msg->get_payload());
            // save the json into the orderBook data structure after acquiring appropriate locks.
            try
            {
                std::string instrument_name = json_msg.at("result").at("instrument_name");
                std::cout << "Instrument Name: " << instrument_name << std::endl;
                {
                    std::lock_guard<std::shared_mutex> lock(order_books_locks[instrument_name]);
                    order_books[instrument_name] = msg->get_payload();
                }
                {
                    std::lock_guard<std::mutex> lock(broadcast_queue_lock);
                    broadcast_queue.push_back(instrument_name);
                }
            }
            catch (nlohmann::json::out_of_range &e)
            {
                std::cerr << "Key not found: " << e.what() << std::endl;
            }

            // Output the received JSON
            std::cout << "\nMessage received from Deribit Test Server (parsed JSON): " << json_msg.dump(4) << std::endl; // Pretty print with 4 spaces indentation
        }
        catch (const nlohmann::json::parse_error &e)
        {
            // If there is a parse error, output the error message
            std::cerr << "JSON parse error: " << e.what() << "\nReceived payload from Deribit test server : " << msg->get_payload() << std::endl;
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

    std::string clientId;
    std::string clientSecret;
    std::string uri;
    client m_client;
    websocketpp::connection_hdl m_hdl;
    std::mutex m_mutex; // Mutex for thread safety of derbit client object, if multiple threads call send_request simultaneously.
};

// Fetch Order Book function
void fetch_order_book(deribit_client &c)
{
    std::cout << "Starting fetch order book thread" << std::endl;

    // Main loop to fetch order books every 10 seconds
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // For each symbol, fetch the order book to be relayed to the connected client.
        for (const auto &itr : subscribed_clients)
        {
            json ob_msg = {
                {"jsonrpc", "2.0"},
                {"id", msg_id},
                {"method", "public/get_order_book"},
                {"params", {{"instrument_name", itr.first}, {"depth", 5}}}};

            std::cout << "OrderBook Fetch request sent: " << ob_msg.dump() << std::endl;
            c.send_request(ob_msg);
        }
    }
}

// broadcast_order_book function
void broadcast_order_book(echoServer &s)
{
    std::cout << "Starting order book broadcasting thread" << std::endl;
    while (true)
    {
        std::string symbol;
        // Lock the queue to get the next symbol to broadcast
        {
            std::lock_guard<std::mutex> lock(broadcast_queue_lock);
            if (!broadcast_queue.empty())
            {
                symbol = broadcast_queue.front();
                broadcast_queue.pop_front();
            }
        }

        if (!symbol.empty())
        {
            // Acquire a shared lock to access the symbol's order book
            std::string symbolOBData = "";
            {
                std::lock_guard<std::shared_mutex> lock(order_books_locks[symbol]);
                symbolOBData = order_books[symbol];
            }

            // Broadcasting data to clients for the symbol
            std::cout << "Broadcasting data to clients for symbol: " << symbol << std::endl;
            for (const auto &client_hdl : subscribed_clients[symbol])
            {
                s.send(client_hdl, symbolOBData, websocketpp::frame::opcode::text);
            }
        }
        // Sleep for 5 seconds.
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

// Function to handle new WebSocket connections
void on_echoClient_open(websocketpp::connection_hdl hdl)
{
    std::cout << __FUNCTION__ << "New client connected : " << std::endl;
}

// Function to handle incoming messages from clients
void on_echoClient_message(echoServer *s, websocketpp::connection_hdl hdl, echoServer::message_ptr msg)
{
    std::cout << __FUNCTION__ << " Message received from client : " << msg->get_payload() << std::endl;

    // Handle subscription requests
    std::string symbol_subscription_req_str = "";
    try
    {
        json json_msg = json::parse(msg->get_payload());
        symbol_subscription_req_str = json_msg.at("subscribe");
        std::cout << " Client symbol subscription request: " << symbol_subscription_req_str << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << "Execption Occured :" << e.what() << std::endl;
    }

    // parse the symbols from subscription string.
    std::stringstream ss(symbol_subscription_req_str);
    std::string field;
    std::vector<std::string> fields;

    // Split the line by commas
    while (std::getline(ss, field, ','))
    {
        fields.push_back(field);
    }

    for (const auto &symb : fields)
    {
        std::lock_guard<std::mutex> lock(subscribed_clients_lock);
        subscribed_clients[symb].push_back(hdl);
        std::cout << "Client subscribed to: " << symb << std::endl;
    }
}

int main()
{
    // Set up local WebSocket server
    echoServer echo_server; // Use the renamed type

    echo_server.set_open_handler(&on_echoClient_open);
    echo_server.set_message_handler(std::bind(&on_echoClient_message, &echo_server, std::placeholders::_1, std::placeholders::_2));
    echo_server.init_asio();
    echo_server.set_reuse_addr(true);
    echo_server.listen(9002);
    echo_server.start_accept();

    // Start the Deribit client and fetch order books
    deribit_client client;
    client.connect();

    // Start fetching orderBook from the connected webSocket
    std::thread orderBookFetchThread(fetch_order_book, std::ref(client));

    // Start broadcasting order books
    std::thread broadcastThread(broadcast_order_book, std::ref(echo_server));

    // Run the WebSocket server in the main thread
    std::cout << "Started running echo server on PORT : 9002" << std::endl;
    echo_server.run();

    // Join threads
    orderBookFetchThread.join();
    broadcastThread.join();

    return 0;
}