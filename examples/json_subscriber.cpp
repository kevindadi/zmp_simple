#include "../include/zmq_simple.hpp"
#include <iostream>
#include <string>
#include <csignal>
#include <atomic>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::atomic<bool> running(true);

void signal_handler(int signal) {
    running = false;
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    try {
        zmq_simple::Subscriber sub("test_json", zmq_simple::Transport::IPC);
        
        std::string topic = "";
        if (argc > 1) {
            topic = argv[1];
            std::cout << "Subscribing to topic: " << topic << std::endl;
        } else {
            std::cout << "Subscribing to all topics" << std::endl;
        }
        sub.subscribe(topic);
        

        while (running) {
            std::string recv_topic;
            std::vector<uint8_t> data;
            
            if (sub.receive(recv_topic, data, 1000)) {
                try {
                    json json_data = json::parse(data.begin(), data.end());
                    
                    std::cout << "Received [" << recv_topic << "]: " << std::endl;
                    std::cout << json_data.dump(2) << std::endl;
                    
                    if (json_data.contains("name")) {
                        std::cout << "  -> Name: " << json_data["name"] << std::endl;
                    }
                    if (json_data.contains("temperature")) {
                        std::cout << "  -> Temperature: " << json_data["temperature"] << "°C" << std::endl;
                    }
                    
                    std::cout << "---" << std::endl;
                } catch (const json::exception& e) {
                    std::cerr << "JSON Parse Error: " << e.what() << std::endl;
                }
            }
        }
        
        /* 
        sub.start_loop([](const std::string& topic, const std::vector<uint8_t>& data) {
            try {
                json json_data = json::parse(data.begin(), data.end());
                std::cout << "收到 [" << topic << "]: " << json_data.dump(2) << std::endl;
            } catch (const json::exception& e) {
                std::cerr << "JSON 解析错误: " << e.what() << std::endl;
            }
        });
        
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        sub.stop_loop();
        */
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

