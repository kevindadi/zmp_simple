/*
 * INPROC 线程间通信机制
 
 * 1. 内存队列机制：
 *    - 底层使用无锁队列（lock-free queue）实现
 *    - 发送方将消息放入队列，接收方从队列取出
 *    - 消息通过指针传递，避免内存拷贝（零拷贝）
 * 
 * 2. 共享 Context 要求：
 *    - 所有 INPROC 端点必须共享同一个 zmq::context_t 对象
 *    - Context 内部维护线程池和 I/O 对象池
 *    - 不同 Context 之间的 INPROC 端点无法通信
 * 
 * 3. 地址绑定：
 *    - 使用 "inproc://名称" 格式的地址
 *    - bind 必须在 connect 之前完成
 *    - 发布者通常 bind，订阅者通常 connect
 * 
 * 【数据有效性保证】
 * 
 * 1. 消息完整性：
 *    - ZMQ 保证消息是原子的，要么完整接收，要么不接收
 *    - 不会出现接收到部分消息的情况
 *    - 消息边界自动维护，无需手动分包
 * 
 * 2. 消息顺序：
 *    - 单个发送者到单个接收者：严格保证顺序（FIFO）
 *    - 多个发送者：各自的消息流内部有序，但总体顺序不保证
 * 
 * 3. 数据安全：
 *    - ZMQ 内部使用引用计数管理消息生命周期
 *    - 消息在所有接收者处理完之前不会被释放
 *    - 线程安全，不需要外部锁
 * 
 * 4. 消息队列限制（HWM - High Water Mark）：
 *    - 默认情况下队列有容量限制（1000 条）
 *    - 队列满时：发送端会阻塞或丢弃消息（取决于 socket 类型）
 *    - PUB socket：满时会丢弃新消息（非阻塞）
 *    - 可通过 set_sndhwm/set_rcvhwm 调整
 * 

 * ============================================================================
 * 典型应用场景：
 * ============================================================================
 * 
 * 1. 【生产者-消费者】: 数据采集线程 -> 处理线程（本示例）
 * 2. 【工作队列】: 主线程 -> 多个工作线程（任务分发）
 * 3. 【事件通知】: 后台线程 -> UI 线程（状态更新）
 * 4. 【数据管道】: 数据源 -> 过滤 -> 转换 -> 输出（流式处理）
 * 5. 【命令控制】: 控制器 -> 执行器（动态配置）
 * 6. 【日志聚合】: 多线程 -> 日志线程（集中管理）
 * 
 * ============================================================================
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
        std::cout << "INPROC 线程间通信示例\n" << std::endl;
        
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

