/*
 * INPROC 线程间通信机制
 * 
 * 典型应用场景：
 *
 * 1. 【生产者-消费者】: 数据采集线程 -> 处理线程
 * 2. 【工作队列】: 主线程 -> 多个工作线程（任务分发）
 *      a.一个线程负责获取传感器数据
 *      b.另一个线程负责处理数据
 *      c.第三个线程可能负责决策或反馈控制

 * 3. 【事件通知】: 后台线程 -> UI 线程
 * 4. 【数据管道】: 数据源 -> 过滤 -> 转换 -> 输出
 * 5. 【命令控制】: 控制命令发布与接收:各个子系统（如飞行控制系统、动力系统等）之间需要通过消息传递进行协调
 *     可以让各个子系统订阅并接收来自其他子系统的命令或数据,保证模块间的松耦合和异步处理.
 * 6. 【日志聚合】: 多线程 -> 日志线程（集中管理）
 * 
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
        
        int count = 0;
        while (running && count < 10) {
            if (count % 3 == 0) {
                std::string sensor_data = "温度: " + std::to_string(20 + count) + "°C";
                pub.publish("sensor", sensor_data);
                std::cout << "[发送] sensor: " << sensor_data << std::endl;
            } else if (count % 3 == 1) {
                std::string status = "系统状态: 正常";
                pub.publish("status", status);
                std::cout << "[发送] status: " << status << std::endl;
            } else {
                std::string log = "完成第 " + std::to_string(count) + " 次采集";
                pub.publish("log", log);
                std::cout << "[发送] log: " << log << std::endl;
            }
            
            count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        pub.publish("control", "结束");
        std::cout << "[发送] control: 结束" << std::endl;
        running = false;
        
    } catch (const std::exception& e) {
        std::cerr << "发送端错误: " << e.what() << std::endl;
    }
}

void subscriber_thread(zmq_simple::Context& ctx) {
    try {
        zmq_simple::Subscriber sub("inproc_channel", zmq_simple::Transport::INPROC, ctx);
        sub.subscribe("");
        
        int sensor_count = 0, status_count = 0, log_count = 0;
        
        sub.start_loop([&](const std::string& topic, const std::vector<uint8_t>& data) {
            std::string msg(data.begin(), data.end());
            std::cout << "[接收] " << topic << ": " << msg << std::endl;
            
            if (topic == "sensor") sensor_count++;
            else if (topic == "status") status_count++;
            else if (topic == "log") log_count++;
        });

        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        sub.stop_loop();
        std::cout << "\n统计: sensor=" << sensor_count 
                  << ", status=" << status_count 
                  << ", log=" << log_count << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "接收端错误: " << e.what() << std::endl;
    }
}

int main() {
    try {
        zmq_simple::Context shared_ctx;
        
        std::thread sub_thread(subscriber_thread, std::ref(shared_ctx));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::thread pub_thread(publisher_thread, std::ref(shared_ctx));
        
        pub_thread.join();
        sub_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

