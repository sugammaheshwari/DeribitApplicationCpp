#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <fstream>
#include <chrono>
#include <nlohmann/json.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <boost/asio.hpp>

using json = nlohmann::json;
typedef websocketpp::client<websocketpp::config::asio_tls_client> client;

std::queue<json> requestQueue;
std::mutex requestQueueLock;
int lastFileSize = 0;
int msg_id = 0; // Global message ID tracker

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

        std::string message = auth_request.dump();
        send_request(auth_request);
        std::cout << "Authentication request sent: " << auth_request.dump() << std::endl;
    }

    void send_request(const json &request_msg)
    {
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
        std::cout << "Message received: " << msg->get_payload() << std::endl;
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
};

void processRequestQueue(deribit_client &c)
{
    std::cout << __FUNCTION__ << std::endl;
    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(requestQueueLock);
            if (!requestQueue.empty())
            {
                json request_msg = requestQueue.front();
                requestQueue.front();
                c.send_request(request_msg);
                std::cout << "Request sent to deribit server from queue: " << request_msg.dump() << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void enqueueBuySellMarketOrder(const std::string &buySell, const std::string &instrument_name, int amount, const std::string &order_type, const std::string &label)
{
    std::cout << __FUNCTION__ << instrument_name << "| |" << buySell << " " << amount << " " << order_type << " " << label << std::endl;
    json order_msg = {
        {"jsonrpc", "2.0"},
        {"id", msg_id++},
        {"method", "private/" + buySell},
        {"params", {{"instrument_name", instrument_name}, {"amount", amount}, {"type", order_type}, {"label", label}}}};

    std::lock_guard<std::mutex> lock(requestQueueLock);
    requestQueue.push(order_msg);
}

void enqueueBuySellLimitOrder(const std::string &buySell, const std::string &instrument_name, int amount, int price, const std::string &order_type, const std::string &label)
{
    json order_msg = {
        {"jsonrpc", "2.0"},
        {"id", msg_id++},
        {"method", "private/" + buySell},
        {"params", {{"instrument_name", instrument_name}, {"amount", amount}, {"price", price}, {"type", order_type}, {"label", label}}}};

    std::lock_guard<std::mutex> lock(requestQueueLock);
    requestQueue.push(order_msg);
}

void enqueueCancelOrder(long long orderId)
{
    json x_msg = {
        {"jsonrpc", "2.0"},
        {"id", msg_id++},
        {"method", "private/cancel"},
        {"params", {{"order_id", orderId}}}};

    std::lock_guard<std::mutex> lock(requestQueueLock);
    requestQueue.push(x_msg);
}

void enqueueModifyOrder(long long orderId, int amount, int price)
{
    json mod_msg = {
        {"jsonrpc", "2.0"},
        {"id", msg_id++},
        {"method", "private/edit"},
        {"params", {{"order_id", orderId}, {"amount", amount}, {"price", price}}}};

    std::lock_guard<std::mutex> lock(requestQueueLock);
    requestQueue.push(mod_msg);
}

void enqueueGetOpenOrders()
{
    json openOrderBook_msg = {
        {"jsonrpc", "2.0"},
        {"id", msg_id++},
        {"method", "private/get_open_orders"}};

    std::lock_guard<std::mutex> lock(requestQueueLock);
    requestQueue.push(openOrderBook_msg);
}

void enqueueGetCurrentPositions()
{
    json pos_msg = {
        {"jsonrpc", "2.0"},
        {"id", msg_id++},
        {"method", "private/get_positions"}};

    std::lock_guard<std::mutex> lock(requestQueueLock);
    requestQueue.push(pos_msg);
}

void fileMonitorThread(const std::string &filePath)
{
    std::cout << __FUNCTION__ << std::endl;

    while (true)
    {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate); // Open the file in binary mode and get size

        if (file)
        {
            std::streampos currentSize = file.tellg();

            // Check if file has grown
            if (currentSize > lastFileSize)
            {
                file.seekg(lastFileSize); // Go to the last read position
                std::string line;

                while (std::getline(file, line))
                {
                    // Process the new lines and add orders to the queue
                    std::cout << "Processing line from file: " << line << std::endl;

                    std::stringstream ss(line);
                    std::string field;
                    std::vector<std::string> fields;

                    // Split the line by commas
                    while (std::getline(ss, field, ','))
                    {
                        fields.push_back(field);
                    }

                    if (fields.size() == 1)
                    {
                        if (fields[0] == "P")
                        {
                            enqueueGetCurrentPositions();
                        }
                        else if (fields[0] == "O")
                        {
                            enqueueGetOpenOrders();
                        }
                    }
                    else if (fields.size() == 6)
                    {
                        enqueueBuySellMarketOrder(fields[1], fields[2], stoi(fields[3]), fields[4], fields[5]);
                    }
                    else if (fields.size() == 7)
                    {
                        enqueueBuySellLimitOrder(fields[1], fields[2], stoi(fields[3]), stoi(fields[4]), fields[5], fields[6]);
                    }
                    else if (fields.size() == 2 && fields[0] == "X")
                    {
                        enqueueCancelOrder(stoll(fields[1]));
                    }
                    else if (fields.size() == 4 && fields[0] == "M")
                    {
                        enqueueModifyOrder(stoll(fields[1]), stoi(fields[2]), stoi(fields[3]));
                    }
                }

                // Update last file size after processing
                lastFileSize = currentSize;
            }
        }

        file.close();                                         // Close the file after reading
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Sleep for 1 second before next iteration
    }
}

int main()
{
    deribit_client client;
    client.connect();

    // Start processing file in a separate thread
    std::thread fileThread(fileMonitorThread, "requests.txt");

    // Start processing request queue in a separate thread
    std::thread requestThread(processRequestQueue, std::ref(client));

    // Join the threads
    fileThread.join();
    requestThread.join();

    return 0;
}
