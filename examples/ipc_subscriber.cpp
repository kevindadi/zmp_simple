#include "../include/zmq_simple.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
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
        zmq_simple::Publisher pub("test_channel", zmq_simple::Transport::IPC);
        
        sub.subscribe("app_a_topic");  
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        int response_count = 0;
        
        while (running) {
            std::string recv_topic;
            std::vector<uint8_t> data;
            
            if (sub.receive(recv_topic, data, 1000)) {
                std::string msg(data.begin(), data.end());
                std::cout << "[接收] " << recv_topic << ": " << msg << std::endl;
                
                std::string response = "来自应用B的响应 #" + std::to_string(response_count);
                pub.publish("app_b_topic", response);
                std::cout << "[发送] app_b_topic: " << response << std::endl;
                
                response_count++;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

