#!/usr/bin/env python3
import sys
import json
import signal
from pathlib import Path

build_path = Path(__file__).parent.parent.parent / "build" / "python"
sys.path.insert(0, str(build_path))

import zmq_simple

running = True

def signal_handler(sig, frame):
    global running
    running = False
    print("\nStopping subscriber...")

def main():
    signal.signal(signal.SIGINT, signal_handler)
    
    sub = zmq_simple.Subscriber("test_json", zmq_simple.Transport.IPC)
    
    topic = sys.argv[1] if len(sys.argv) > 1 else ""
    if topic:
        print(f"Subscribing to topic: {topic}")
    else:
        print("Subscribing to all topics")
    
    sub.subscribe(topic)

    try:
        while running:
            recv_topic, data = sub.receive(1000)
            
            if recv_topic:
                try:
                    json_data = json.loads(data.decode('utf-8'))
                    
                    print(f"Received [{recv_topic}]:")
                    print(json.dumps(json_data, indent=2))
                    
                    if "name" in json_data:
                        print(f"  -> Name: {json_data['name']}")
                    if "temperature" in json_data:
                        print(f"  -> Temperature: {json_data['temperature']}°C")
                    
                    print("---")
                    
                except json.JSONDecodeError as e:
                    print(f"JSON Parse Error: {e}")
                    
    except Exception as e:
        print(f"Error: {e}")
    
    print("Subscriber stopped")

if __name__ == "__main__":
    main()

