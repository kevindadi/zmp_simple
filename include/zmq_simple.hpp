#ifndef ZMQ_SIMPLE_HPP
#define ZMQ_SIMPLE_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace zmq_simple {

enum class Transport {
    IPC,     
    INPROC 
};

class Context {
public:
    Context();
    ~Context();

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    
    void* get_raw_context();
    
private:
    void* context_;
};

class Publisher {
public:
    Publisher(const std::string& endpoint, Transport transport = Transport::IPC);
    Publisher(const std::string& endpoint, Transport transport, Context& shared_context);

    ~Publisher();

    Publisher(const Publisher&) = delete;
    Publisher& operator=(const Publisher&) = delete;

    bool publish(const std::string& topic, const std::string& data);
    bool publish(const std::string& topic, const void* data, size_t size);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

class Subscriber {
public:
    using MessageCallback = std::function<void(const std::string& topic, const std::vector<uint8_t>& data)>;

    Subscriber(const std::string& endpoint, Transport transport = Transport::IPC); 
    Subscriber(const std::string& endpoint, Transport transport, Context& shared_context);
    
    ~Subscriber();

    Subscriber(const Subscriber&) = delete;
    Subscriber& operator=(const Subscriber&) = delete;

    bool subscribe(const std::string& topic = "");  
    bool unsubscribe(const std::string& topic);

    bool receive(std::string& topic, std::vector<uint8_t>& data, int timeout_ms = -1);
    
    bool start_loop(MessageCallback callback);
    void stop_loop();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace zmq_simple

#endif // ZMQ_SIMPLE_HPP

