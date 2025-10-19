#include "../include/zmq_simple.hpp"
#include <zmq.h>
#include <thread>
#include <atomic>
#include <cstring>
namespace zmq_simple {

Context::Context() {
    context_ = zmq_ctx_new();
    if (!context_) {
        throw std::runtime_error("Failed to create ZeroMQ context");
    }
}

Context::~Context() {
    if (context_) {
        zmq_ctx_destroy(context_);
    }
}

void* Context::get_raw_context() {
    return context_;
}

class Publisher::Impl {
public:
    Impl(const std::string& endpoint, Transport transport) 
        : owns_context_(true) {
        context_ = zmq_ctx_new();
        socket_ = zmq_socket(context_, ZMQ_PUB);
        
        std::string addr = build_address(endpoint, transport);
        
        if (zmq_bind(socket_, addr.c_str()) != 0) {
            throw std::runtime_error("Failed to bind publisher: " + std::string(zmq_strerror(zmq_errno())));
        }
    }
    
    Impl(const std::string& endpoint, Transport transport, void* shared_context)
        : owns_context_(false), context_(shared_context) {
        socket_ = zmq_socket(context_, ZMQ_PUB);
        
        std::string addr = build_address(endpoint, transport);
        
        if (zmq_bind(socket_, addr.c_str()) != 0) {
            throw std::runtime_error("Failed to bind publisher: " + std::string(zmq_strerror(zmq_errno())));
        }
    }
    
    ~Impl() {
        if (socket_) {
            zmq_close(socket_);
        }
        if (context_ && owns_context_) {
            zmq_ctx_destroy(context_);
        }
    }
    
    bool publish(const std::string& topic, const void* data, size_t size) {
        // 先发送Topic
        if (zmq_send(socket_, topic.c_str(), topic.size(), ZMQ_SNDMORE) == -1) {
            return false;
        }
        
        // 再发送Data
        if (zmq_send(socket_, data, size, 0) == -1) {
            return false;
        }
        
        return true;
    }
    
private:
    std::string build_address(const std::string& endpoint, Transport transport) {
        if (transport == Transport::IPC) {
            return "ipc:///tmp/docker_share/" + endpoint + ".ipc";
        } else {
            return "inproc://" + endpoint;
        }
    }
    
    bool owns_context_;
    void* context_;
    void* socket_;
};

Publisher::Publisher(const std::string& endpoint, Transport transport)
    : pimpl_(std::make_unique<Impl>(endpoint, transport)) {
}

Publisher::Publisher(const std::string& endpoint, Transport transport, Context& shared_context)
    : pimpl_(std::make_unique<Impl>(endpoint, transport, shared_context.get_raw_context())) {
}

Publisher::~Publisher() = default;

bool Publisher::publish(const std::string& topic, const std::string& data) {
    return pimpl_->publish(topic, data.c_str(), data.size());
}

bool Publisher::publish(const std::string& topic, const void* data, size_t size) {
    return pimpl_->publish(topic, data, size);
}

class Subscriber::Impl {
public:
    Impl(const std::string& endpoint, Transport transport) 
        : running_(false), owns_context_(true) {
        context_ = zmq_ctx_new();
        socket_ = zmq_socket(context_, ZMQ_SUB);
        
        const std::string addr = build_address(endpoint, transport);
        
        if (zmq_connect(socket_, addr.c_str()) != 0) {
            throw std::runtime_error("Failed to connect subscriber: " + std::string(zmq_strerror(zmq_errno())));
        }
    }
    
    Impl(const std::string& endpoint, Transport transport, void* shared_context)
        : running_(false), owns_context_(false), context_(shared_context) {
        socket_ = zmq_socket(context_, ZMQ_SUB);
        
        std::string addr = build_address(endpoint, transport);
        
        if (zmq_connect(socket_, addr.c_str()) != 0) {
            throw std::runtime_error("Failed to connect subscriber: " + std::string(zmq_strerror(zmq_errno())));
        }
    }
    
    ~Impl() {
        stop_loop();
        
        if (socket_) {
            zmq_close(socket_);
        }
        if (context_ && owns_context_) {
            zmq_ctx_destroy(context_);
        }
    }
    
    bool subscribe(const std::string& topic) {
        return zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, topic.c_str(), topic.size()) == 0;
    }
    
    bool unsubscribe(const std::string& topic) {
        return zmq_setsockopt(socket_, ZMQ_UNSUBSCRIBE, topic.c_str(), topic.size()) == 0;
    }
    
    bool receive(std::string& topic, std::vector<uint8_t>& data, int timeout_ms) {
        if (timeout_ms >= 0) {
            zmq_setsockopt(socket_, ZMQ_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));
        }
        
        // 接收Topic
        zmq_msg_t topic_msg;
        zmq_msg_init(&topic_msg);
        
        int rc = zmq_msg_recv(&topic_msg, socket_, 0);
        if (rc == -1) {
            zmq_msg_close(&topic_msg);
            if (zmq_errno() == EAGAIN) {
                return false; // Timeout
            }
            return false;
        }
        
        topic.assign(static_cast<const char*>(zmq_msg_data(&topic_msg)), zmq_msg_size(&topic_msg));
        zmq_msg_close(&topic_msg);
        
        // 接收Data
        zmq_msg_t data_msg;
        zmq_msg_init(&data_msg);
        
        rc = zmq_msg_recv(&data_msg, socket_, 0);
        if (rc == -1) {
            zmq_msg_close(&data_msg);
            return false;
        }
        
        const uint8_t* msg_data = static_cast<const uint8_t*>(zmq_msg_data(&data_msg));
        size_t msg_size = zmq_msg_size(&data_msg);
        data.assign(msg_data, msg_data + msg_size);
        
        zmq_msg_close(&data_msg);
        
        return true;
    }
    
    bool start_loop(MessageCallback callback) {
        if (running_) {
            return false;
        }
        
        running_ = true;
        thread_ = std::thread([this, callback]() {
            while (running_) {
                std::string topic;
                std::vector<uint8_t> data;
                
                if (receive(topic, data, 100)) { // 100ms 超时以检查 running_ 标志
                    callback(topic, data);
                }
            }
        });
        
        return true;
    }
    
    void stop_loop() {
        if (running_) {
            running_ = false;
            if (thread_.joinable()) {
                thread_.join();
            }
        }
    }

private:
    static std::string build_address(const std::string& endpoint, const Transport transport) {
        if (transport == Transport::IPC) {
            return "ipc:///tmp/docker_share/" + endpoint + ".ipc";
        } else {
            return "inproc://" + endpoint;
        }
    }
    
    bool owns_context_;
    void* context_;
    void* socket_;
    std::atomic<bool> running_;
    std::thread thread_;
};

Subscriber::Subscriber(const std::string& endpoint, Transport transport)
    : pimpl_(std::make_unique<Impl>(endpoint, transport)) {
}

Subscriber::Subscriber(const std::string& endpoint, Transport transport, Context& shared_context)
    : pimpl_(std::make_unique<Impl>(endpoint, transport, shared_context.get_raw_context())) {
}

Subscriber::~Subscriber() = default;

bool Subscriber::subscribe(const std::string& topic) {
    return pimpl_->subscribe(topic);
}

bool Subscriber::unsubscribe(const std::string& topic) {
    return pimpl_->unsubscribe(topic);
}

bool Subscriber::receive(std::string& topic, std::vector<uint8_t>& data, int timeout_ms) {
    return pimpl_->receive(topic, data, timeout_ms);
}

bool Subscriber::start_loop(MessageCallback callback) {
    return pimpl_->start_loop(callback);
}

void Subscriber::stop_loop() {
    pimpl_->stop_loop();
}


} // namespace zmq_simple

