/**
 * @file ipc_publisher.cpp
 * @brief IPC 发布者示例
 */

#include "../include/zmq_simple.hpp"
#include <iostream>
#include <thread>
#include <chrono>   

int main() {    
    try {
        zmq_simple::Publisher pub("test_channel", zmq_simple::Transport::IPC);
        
        int count = 0;
        while (true) {
            std::string msg1 = "Hello from topic1 - " + std::to_string(count);
            pub.publish("topic1", msg1);
            std::cout << "Published: [topic1] " << msg1 << std::endl;
            
            std::string msg2 = "Hello from topic2 - " + std::to_string(count);
            pub.publish("topic2", msg2);
            std::cout << "Published: [topic2] " << msg2 << std::endl;
            
            count++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

