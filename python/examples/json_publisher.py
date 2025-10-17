#!/usr/bin/env python3
import sys
import time
import json
from pathlib import Path

# 添加构建目录到 Python 路径
build_path = Path(__file__).parent.parent.parent / "build" / "python"
sys.path.insert(0, str(build_path))

import zmq_simple

def main():
    pub = zmq_simple.Publisher("test_json", zmq_simple.Transport.IPC)
    print("Publisher started on 'test_json' channel")
    
    count = 0
    try:
        while True:
            user_data = {
                "id": count,
                "name": f"User_{count}",
                "age": 20 + (count % 50),
                "email": f"user{count}@example.com",
                "timestamp": int(time.time())
            }
            
            user_json = json.dumps(user_data)
            pub.publish("user", user_json)
            print(f"Published [user]: {json.dumps(user_data, indent=2)}")
            
            sensor_data = {
                "sensor_id": "temp_sensor_01",
                "temperature": 20.0 + (count % 10) * 0.5,
                "humidity": 50.0 + (count % 20) * 1.5,
                "timestamp": int(time.time())
            }

            sensor_json = json.dumps(sensor_data)
            pub.publish("sensor", sensor_json)
            print(f"Published [sensor]: {json.dumps(sensor_data, indent=2)}")
            
            print("---")
            count += 1
            time.sleep(2)
            
    except KeyboardInterrupt:
        print("\nPublisher stopped")

if __name__ == "__main__":
    main()

