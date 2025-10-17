# IPC Publisher
add_executable(ipc_publisher ipc_publisher.cpp)
target_link_libraries(ipc_publisher zmq_simple)

# IPC Subscriber
add_executable(ipc_subscriber ipc_subscriber.cpp)
target_link_libraries(ipc_subscriber zmq_simple)

# INPROC Example
add_executable(inproc_example inproc_example.cpp)
target_link_libraries(inproc_example zmq_simple pthread)

include_directories(/opt/homebrew/include)
find_package(nlohmann_json REQUIRED)
add_executable(json_publisher json_publisher.cpp)
target_link_libraries(json_publisher zmq_simple nlohmann_json::nlohmann_json)

add_executable(json_subscriber json_subscriber.cpp)
target_link_libraries(json_subscriber zmq_simple nlohmann_json::nlohmann_json)

