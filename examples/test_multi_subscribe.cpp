#include "../include/zmq_simple.hpp"
#include <iostream>
#include <thread>
#include <chrono>

void publisher_func() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    zmq_simple::Publisher pub("test_multi", zmq_simple::Transport::IPC);
    
    for (int i = 0; i < 3; i++) {
        pub.publish("topic_a", "Message A-" + std::to_string(i));
        std::cout << "[发布] topic_a: Message A-" << i << std::endl;
        
        pub.publish("topic_b", "Message B-" + std::to_string(i));
        std::cout << "[发布] topic_b: Message B-" << i << std::endl;
        
        pub.publish("topic_c", "Message C-" + std::to_string(i));
        std::cout << "[发布] topic_c: Message C-" << i << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main() {
    std::thread pub_thread(publisher_func);
    
    try {
        zmq_simple::Subscriber sub("test_multi", zmq_simple::Transport::IPC);
             
        std::cout << "订阅 topic_c..." << std::endl;
        sub.subscribe("topic_c");

        std::cout << "订阅 topic_a..." << std::endl;
        sub.subscribe("topic_a");
        
        std::cout << "订阅 topic_b..." << std::endl;
        sub.subscribe("topic_b");
   
        
        std::cout << "\n开始接收消息...\n" << std::endl;
        
        int count = 0;
        int max_count = 9;
        
        while (count < max_count) {
            std::string topic;
            std::vector<uint8_t> data;
            
            if (sub.receive(topic, data, 3000)) {
                std::string msg(data.begin(), data.end());
                std::cout << "[接收] " << topic << ": " << msg << std::endl;
                count++;
            } else {
                std::cout << "接收超时" << std::endl;
                break;
            }
        }
        
        std::cout << "\n总共接收到 " << count << " 条消息（预期 " << max_count << " 条）" << std::endl;
        
        if (count == max_count) {
            std::cout << "✓ 测试通过：单个 Subscriber 可以订阅多个主题" << std::endl;
        } else {
            std::cout << "✗ 测试失败：未收到所有消息" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        pub_thread.join();
        return 1;
    }
    
    pub_thread.join();
    return 0;
}

