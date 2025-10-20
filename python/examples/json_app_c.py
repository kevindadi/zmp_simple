#!/usr/bin/env python3
from pprint import pprint
import sys
import json
import time
import signal
from pathlib import Path

build_path = Path(__file__).parent.parent.parent / "build" / "python"
sys.path.insert(0, str(build_path))

import zmq_simple

running = True

def signal_handler(sig, frame):
    global running
    running = False

def main():
    global running
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    try:
        pub = zmq_simple.Publisher("C_publisher", zmq_simple.Transport.IPC)
        sub_a = zmq_simple.Subscriber("A_publisher", zmq_simple.Transport.IPC)
        sub_b = zmq_simple.Subscriber("B_publisher", zmq_simple.Transport.IPC)
        
        sub_a.subscribe("")
        sub_b.subscribe("")
        
        def on_message_a(topic, data):
            try:
                received = json.loads(data.decode('utf-8'))
                print(f"[A→C] {json.dumps(received)}")
            except Exception as e:
                print(f"错误: {e}")
        
        def on_message_b(topic, data):
            try:
                received = json.loads(data.decode('utf-8'))
                print(f"[B→C] {json.dumps(received)}")
            except Exception as e:
                print(f"错误: {e}")
        
        sub_a.start_loop(on_message_a)
        sub_b.start_loop(on_message_b)
        
        toBcount = 0
        toAcount = 0
        while running:
            time.sleep(3)
            messageA = {"name": "C", "message": "C to A", "count": toAcount}
            pub.publish("CA", json.dumps(messageA))
            print(f"[C Publish to A:] {json.dumps(messageA)}")
            toAcount += 1
            messageB = {"name": "C", "message": "C to B", "count": toBcount}
            pub.publish("CB", json.dumps(messageB))
            print(f"[C Publish to B:] {json.dumps(messageB)}")
            toBcount += 1

        sub_a.stop_loop()
        sub_b.stop_loop()
    
    except Exception as e:
        print(f"错误: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())

