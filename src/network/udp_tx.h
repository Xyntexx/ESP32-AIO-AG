#ifndef UDP_TX_H
#define UDP_TX_H

#include <stddef.h>
#include <stdint.h>

namespace udp_tx {

// Logical send channels. Each channel resolves to a registered sender
// function that the drainer task invokes when it pops a queued payload.
enum class Channel : uint8_t {
    Autosteer = 0,
    GPS       = 1,
    _COUNT
};

using SendFn = bool (*)(const uint8_t* data, size_t len);

struct Stats {
    uint32_t enqueued;
    uint32_t dropped_full;     // queue was full, oldest descriptor was evicted
    uint32_t dropped_oversize; // payload exceeded UDP_TX_MAX_PAYLOAD
    uint32_t dropped_unreg;    // channel had no sender registered
    uint32_t sent_ok;
    uint32_t send_failed;      // sender returned false
    uint32_t last_drain_ms;    // millis() when the drainer last completed a send
    uint16_t depth_high_water;
};

// Create the queue + spawn the drainer task. Idempotent. Must be called
// before any producer enqueues - call as early as possible in setup(),
// before initUDP() and before tasks that send (gps_task, autoSteerTask).
bool init();

// Wire the underlying send function for a channel. Channels with no
// registered sender at drain time get counted as dropped_unreg and
// silently skipped - the drainer never blocks waiting for one.
void registerChannel(Channel ch, SendFn fn);

// Non-blocking enqueue. Safe from any context (lwIP RX callback, 1 kHz
// task, GPS UART loop). Returns true when the descriptor is queued; on
// a full queue the oldest descriptor is dropped and the new one queued
// (the bumped dropped_full counter records this). Returns false only
// when the payload is too large or the module is uninitialized.
bool enqueue(Channel ch, const uint8_t* data, size_t len);

// Snapshot of telemetry counters. Reads are atomic per-field on Xtensa
// (32-bit aligned uint32), so a snapshot may straddle a sender update,
// but each field is internally consistent.
Stats stats();

// Shorthand for stats().last_drain_ms - cheap polling for the safety
// task in PR2 to detect a wedged TX path.
uint32_t lastDrainMillis();

} // namespace udp_tx

#endif // UDP_TX_H
