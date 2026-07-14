/**
 * cam678_example.cpp — C++ equivalent of cam678_example.py
 * Uses Hololink directly (no DataChannel) — 2.5.0 compatible.
 *
 * Compile:
 *   g++ -std=c++17 -O2 \
 *       -I /usr/local/include \
 *       -I /usr/local/include/hololink/core \
 *       -I src/ \
 *       -I /usr/local/cuda/include \
 *       examples/cam678_example.cpp \
 *       build/src/hololink/sensors/camera/cam678/libnative_cam678_camera_sensor.a \
 *       build/src/hololink/sensors/camera/libhololink_camera_sensor.a \
 *       build/src/hololink/sensors/libhololink_sensor.a \
 *       /usr/local/lib/libhololink_core.a \
 *       -L /usr/local/cuda/lib64 \
 *       -lpthread -lcuda -ldl -lrt \
 *       -o cam678_example
 *
 * Run:
 *   ./cam678_example
 */

#include <cstdio>
#include <memory>
#include <thread>
#include <chrono>

#include <hololink/core/hololink.hpp>
#include <hololink/core/enumerator.hpp>
#include <hololink/core/metadata.hpp>
#include <hololink/sensors/camera/cam678/native_cam678_sensor.hpp>

using namespace hololink;
using namespace hololink::sensors;

static constexpr char EMULATOR_IP[] = "192.168.0.2";

int main()
{
    printf("=== cam678 C++ Example ===\n\n");

    // ── 1. Discover emulator ──────────────────────────────────────────────────
    printf("[host] Waiting for emulator on %s ...\n", EMULATOR_IP);
    Metadata metadata = Enumerator::find_channel(EMULATOR_IP);
    printf("[host] Found.\n");

    // ── 2. Get Hololink handle (same as i2c_host.cpp — no DataChannel) ────────
    auto hololink = Hololink::from_enumeration_metadata(metadata);
    hololink->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    try {
        hololink->reset();
        printf("[host] reset() OK\n");
    } catch (const std::exception& e) {
        printf("[host] reset() skipped: %s\n", e.what());
    }

    // ── 3. Create cam678 sensor ───────────────────────────────────────────────
    auto cam = std::make_shared<NativeCam678Sensor>(hololink);

    // ── 4. Call all methods ───────────────────────────────────────────────────
    uint32_t version = cam->get_version();
    printf("\nVersion: 0x%04X\n\n", version);

    cam->configure(0);
    cam->set_gain(42);
    cam->set_exposure(100);
    cam->start();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    cam->stop();

    // ── 5. Cleanup ────────────────────────────────────────────────────────────
    hololink->stop();
    printf("\n=== Done ===\n");
    return 0;
}