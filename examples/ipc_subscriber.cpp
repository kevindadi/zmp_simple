/**
 * @file ipc_subscriber.cpp
 * @brief IPC 订阅者示例
 */

#include "../include/zmq_simple.hpp"
#include <iostream>
#include <string>
#include <csignal>
#include <atomic>

std::atomic<bool> running(true);

void signal_handler(int signal) {
    running = false;
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    try {
        zmq_simple::Subscriber sub("test_channel", zmq_simple::Transport::IPC);
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
                std::string msg(data.begin(), data.end());
                std::cout << "Received: [" << recv_topic << "] " << msg << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

