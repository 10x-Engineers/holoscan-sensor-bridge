#!/usr/bin/env python3
import hololink as hololink_module
from hololink.sensors.camera.cam380 import NativeCam380Sensor

EMULATOR_IP = "192.168.0.2"

def main():
    print("=== cam380 Example (baked in hololink) ===\n")

    metadata = hololink_module.Enumerator.find_channel(channel_ip=EMULATOR_IP)
    hololink_channel = hololink_module.DataChannel(metadata)
    hololink_node = hololink_channel.hololink()
    hololink_node.start()

    cam = NativeCam380Sensor(hololink_node)

    print(f"Version: 0x{cam.get_version():04X}\n")
    cam.configure(0)
    cam.set_analog_gain(16)
    cam.set_digital_gain(256)
    cam.set_exposure(500)
    cam.print_status()
    cam.start()
    cam.stop()

    hololink_node.stop()
    print("\n=== Done ===")

if __name__ == "__main__":
    main()