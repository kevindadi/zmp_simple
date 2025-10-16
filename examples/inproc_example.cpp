/**
 * @file inproc_example.cpp
 * @brief INPROC 进程内通信示例
 */

#include "../include/zmq_simple.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

std::atomic<bool> running(true);

void publisher_thread(zmq_simple::Context& ctx) {
    try {
        zmq_simple::Publisher pub("inproc_channel", zmq_simple::Transport::INPROC, ctx);
        std::cout << "[Publisher] Started" << std::endl;
      
        int count = 0;
        while (running && count < 10) {
            std::string msg = "Message " + std::to_string(count);
            pub.publish("test", msg);
            std::cout << "[Publisher] Sent: " << msg << std::endl;
            
            count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        std::cout << "[Publisher] Finished" << std::endl;
        running = false;
        
    } catch (const std::exception& e) {
        std::cerr << "[Publisher] Error: " << e.what() << std::endl;
    }
}

void subscriber_thread(zmq_simple::Context& ctx) {
    try {
        zmq_simple::Subscriber sub("inproc_channel", zmq_simple::Transport::INPROC, ctx);
        sub.subscribe("");
        std::cout << "[Subscriber] Started and subscribed to all topics" << std::endl;
        
        sub.start_loop([](const std::string& topic, const std::vector<uint8_t>& data) {
            std::string msg(data.begin(), data.end());
            std::cout << "[Subscriber] Received: [" << topic << "] " << msg << std::endl;
        });

        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        sub.stop_loop();
        std::cout << "[Subscriber] Finished" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[Subscriber] Error: " << e.what() << std::endl;
    }
}

int main() {
    try {
        zmq_simple::Context shared_ctx;
        std::thread sub_thread(subscriber_thread, std::ref(shared_ctx));
        std::thread pub_thread(publisher_thread, std::ref(shared_ctx));
        
        pub_thread.join();
        sub_thread.join();    
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

