#!/usr/bin/env python3
import hololink as hololink_module
from hololink.sensors.camera.cam678 import NativeCam678Sensor

EMULATOR_IP = "192.168.0.2"

def main():
    print("=== cam678 Example ===\n")

    metadata = hololink_module.Enumerator.find_channel(channel_ip=EMULATOR_IP)
    hololink_channel = hololink_module.DataChannel(metadata)
    hololink_node = hololink_channel.hololink()
    hololink_node.start()

    # Pass hololink_node directly (not hololink_channel)
    cam = NativeCam678Sensor(hololink_node)

    version = cam.get_version()
    print(f"\nVersion: 0x{version:04X}")

    cam.configure(0)
    cam.set_gain(42)
    cam.set_exposure(100)
    cam.start()
    cam.stop()

    hololink_node.stop()
    print("\n=== Done ===")

if __name__ == "__main__":
    main()
