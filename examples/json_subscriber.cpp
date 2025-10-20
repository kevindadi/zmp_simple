#include "../include/zmq_simple.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::atomic<bool> running(true);
void signal_handler(int signal)
{
    running = false;
}

int main(int argc, char *argv[])
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try
    {
        zmq_simple::Subscriber sub_a("A_publisher", zmq_simple::Transport::IPC);
        zmq_simple::Subscriber sub_c("C_publisher", zmq_simple::Transport::IPC);
        zmq_simple::Publisher pub("B_publisher", zmq_simple::Transport::IPC);

        sub_a.subscribe("");
        sub_c.subscribe("CB");

        sub_a.start_loop([&pub](const std::string &topic, const std::vector<uint8_t> &data)
                         {
            try {
                json received = json::parse(data.begin(), data.end());
                std::cout << "[A→B] " << received.dump() << std::endl;
            } catch (const json::exception& e) {
                std::cerr << "JSON 解析错误: " << e.what() << std::endl;
            } });

        sub_c.start_loop([&pub](const std::string &topic, const std::vector<uint8_t> &data)
                         {
            try {
                json received = json::parse(data.begin(), data.end());
                std::cout << "[C→B] " << received.dump() << std::endl;
            } catch (const json::exception& e) {
                std::cerr << "JSON 解析错误: " << e.what() << std::endl;
            } });

        int count = 0;
        while (running)
        {
            json response = {
                {"name", "B"},
                {"message", "B to ALL"},
                {"count", std::to_string(count)}};
            std::string json_str = response.dump();
            pub.publish("app_b_data", json_str);
            std::cout << "[B Publish:] " << json_str << std::endl;
            count++;
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
        sub_a.stop_loop();
        sub_c.stop_loop();
    }
    catch (const std::exception &e)
    {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
