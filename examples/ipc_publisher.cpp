#include "../include/zmq_simple.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

std::atomic<bool> running(true);

void signal_handler(int signal) {
    running = false;
}

int main() {    
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    try {
        zmq_simple::Publisher pub("test_channel", zmq_simple::Transport::IPC);
        zmq_simple::Subscriber sub("test_channel", zmq_simple::Transport::IPC);
        
        sub.subscribe("app_b_topic");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        int count = 0;
        while (running) {
            std::string msg = "来自应用A的消息 #" + std::to_string(count);
            pub.publish("app_a_topic", msg);
            std::cout << "[发送] app_a_topic: " << msg << std::endl;
            
            std::string recv_topic;
            std::vector<uint8_t> data;
            if (sub.receive(recv_topic, data, 500)) {
                std::string received_msg(data.begin(), data.end());
                std::cout << "[收到] " << recv_topic << ": " << received_msg << std::endl;
            }
            
            count++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }       
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

