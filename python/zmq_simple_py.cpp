#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include "../include/zmq_simple.hpp"

namespace py = pybind11;

PYBIND11_MODULE(zmq_simple, m) {
    m.doc() = "Simple ZeroMQ pub-sub wrapper for Python";

    py::enum_<zmq_simple::Transport>(m, "Transport")
        .value("IPC", zmq_simple::Transport::IPC)
        .value("INPROC", zmq_simple::Transport::INPROC)
        .export_values();

    // 上下文
    py::class_<zmq_simple::Context>(m, "Context")
        .def(py::init<>());

    // Publisher
    py::class_<zmq_simple::Publisher>(m, "Publisher")
        .def(py::init<const std::string&, zmq_simple::Transport>(),
             py::arg("endpoint"),
             py::arg("transport") = zmq_simple::Transport::IPC)
        .def(py::init<const std::string&, zmq_simple::Transport, zmq_simple::Context&>(),
             py::arg("endpoint"),
             py::arg("transport"),
             py::arg("shared_context"))
        .def("publish", 
             static_cast<bool (zmq_simple::Publisher::*)(const std::string&, const std::string&)>(
                 &zmq_simple::Publisher::publish),
             py::arg("topic"),
             py::arg("data"),
             "Publish a message to a topic")
        .def("publish_bytes",
             [](zmq_simple::Publisher& self, const std::string& topic, const py::bytes& data) {
                 std::string str_data = data;
                 return self.publish(topic, str_data.c_str(), str_data.size());
             },
             py::arg("topic"),
             py::arg("data"),
             "Publish binary data to a topic");

    // Subscriber
    py::class_<zmq_simple::Subscriber>(m, "Subscriber")
        .def(py::init<const std::string&, zmq_simple::Transport>(),
             py::arg("endpoint"),
             py::arg("transport") = zmq_simple::Transport::IPC)
        .def(py::init<const std::string&, zmq_simple::Transport, zmq_simple::Context&>(),
             py::arg("endpoint"),
             py::arg("transport"),
             py::arg("shared_context"))
        .def("subscribe",
             &zmq_simple::Subscriber::subscribe,
             py::arg("topic") = "",
             "Subscribe to a topic (empty string subscribes to all topics)")
        .def("unsubscribe",
             &zmq_simple::Subscriber::unsubscribe,
             py::arg("topic"),
             "Unsubscribe from a topic")
        .def("receive",
             [](zmq_simple::Subscriber& self, int timeout_ms) {
                 std::string topic;
                 std::vector<uint8_t> data;
                 bool success = self.receive(topic, data, timeout_ms);
                 if (success) {
                     return py::make_tuple(topic, py::bytes(reinterpret_cast<const char*>(data.data()), data.size()));
                 } else {
                     return py::make_tuple(py::str(), py::bytes());
                 }
             },
             py::arg("timeout_ms") = -1,
             "Receive a message (returns tuple of (topic, data) or (None, None) on timeout)")
        .def("start_loop",
             [](zmq_simple::Subscriber& self, py::function callback) {
                 return self.start_loop([callback](const std::string& topic, const std::vector<uint8_t>& data) {
                     py::gil_scoped_acquire acquire;
                     callback(topic, py::bytes(reinterpret_cast<const char*>(data.data()), data.size()));
                 });
             },
             py::arg("callback"),
             "Start asynchronous message loop with callback")
        .def("stop_loop",
             &zmq_simple::Subscriber::stop_loop,
             "Stop the message loop");
}

