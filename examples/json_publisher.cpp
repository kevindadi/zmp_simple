#include "../include/zmq_simple.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {    
    try {
        zmq_simple::Publisher pub("test_json", zmq_simple::Transport::IPC);
        
        int count = 0;
        while (true) {
            json user_data = {
                {"id", count},
                {"name", "User_" + std::to_string(count)},
                {"age", 20 + (count % 50)},
                {"email", "user" + std::to_string(count) + "@example.com"},
                {"timestamp", std::time(nullptr)}
            };

            std::string user_json_str = user_data.dump();
            pub.publish("user", user_json_str);
            std::cout << "Published [user]: " << user_data.dump(2) << std::endl;

            json sensor_data = {
                {"sensor_id", "temp_sensor_01"},
                {"temperature", 20.0 + (count % 10) * 0.5},
                {"humidity", 50.0 + (count % 20) * 1.5},
                {"timestamp", std::time(nullptr)}
            };

            std::string sensor_json_str = sensor_data.dump();
            pub.publish("sensor", sensor_json_str);
            std::cout << "Published [sensor]: " << sensor_data.dump(2) << std::endl;

            std::cout << "---" << std::endl;
            count++;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

