#include "udp_tx.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <string.h>

#include "config/defines.h"
#include "utils/log.h"

namespace udp_tx {

namespace {

struct Descriptor {
    uint8_t  channel;
    uint16_t len;
    uint8_t  payload[UDP_TX_MAX_PAYLOAD];
};

constexpr size_t CHANNEL_COUNT = static_cast<size_t>(Channel::_COUNT);

// Static queue storage: avoids heap allocation for ~8 KB of buffer.
StaticQueue_t       s_queueCb;
uint8_t             s_queueStorage[UDP_TX_QUEUE_DEPTH * sizeof(Descriptor)];
QueueHandle_t       s_queue       = nullptr;

StaticTask_t        s_taskCb;
StackType_t         s_taskStack[4096];
TaskHandle_t        s_task        = nullptr;

SendFn              s_senders[CHANNEL_COUNT] = {nullptr};
volatile Stats      s_stats                  = {};

bool                s_inited = false;

void drainer(void*) {
    Descriptor d;
    for (;;) {
        if (xQueueReceive(s_queue, &d, portMAX_DELAY) != pdTRUE) continue;

        if (d.channel >= CHANNEL_COUNT) {
            // Defensive - corrupt descriptor; count and skip.
            s_stats.send_failed++;
            continue;
        }

        SendFn fn = s_senders[d.channel];
        if (!fn) {
            s_stats.dropped_unreg++;
            continue;
        }

        if (fn(d.payload, d.len)) {
            s_stats.sent_ok++;
            s_stats.last_drain_ms = millis();
        } else {
            s_stats.send_failed++;
        }
    }
}

} // namespace

bool init() {
    if (s_inited) return true;

    s_queue = xQueueCreateStatic(
        UDP_TX_QUEUE_DEPTH,
        sizeof(Descriptor),
        s_queueStorage,
        &s_queueCb);
    if (!s_queue) {
        error("udp_tx: queue create failed");
        return false;
    }

    s_task = xTaskCreateStatic(
        drainer,
        "udp_tx",
        sizeof(s_taskStack) / sizeof(StackType_t),
        nullptr,
        UDP_TX_TASK_PRIORITY,
        s_taskStack,
        &s_taskCb);
    if (!s_task) {
        error("udp_tx: task create failed");
        return false;
    }

    s_inited = true;
    infof("udp_tx: started (depth=%d, payload=%d, prio=%d)",
          UDP_TX_QUEUE_DEPTH, UDP_TX_MAX_PAYLOAD, UDP_TX_TASK_PRIORITY);
    return true;
}

void registerChannel(Channel ch, SendFn fn) {
    size_t i = static_cast<size_t>(ch);
    if (i < CHANNEL_COUNT) {
        s_senders[i] = fn;
    }
}

bool enqueue(Channel ch, const uint8_t* data, size_t len) {
    if (!s_inited || !s_queue) return false;
    if (len > UDP_TX_MAX_PAYLOAD) {
        s_stats.dropped_oversize++;
        return false;
    }

    Descriptor d;
    d.channel = static_cast<uint8_t>(ch);
    d.len     = static_cast<uint16_t>(len);
    memcpy(d.payload, data, len);

    // Non-blocking. On full, evict oldest and retry once - we'd rather
    // a stale descriptor go than make a producer wait for the network.
    if (xQueueSend(s_queue, &d, 0) != pdTRUE) {
        Descriptor evict;
        xQueueReceive(s_queue, &evict, 0);
        s_stats.dropped_full++;
        if (xQueueSend(s_queue, &d, 0) != pdTRUE) {
            // Should never happen on a single-producer-per-channel path,
            // but keep the failure observable rather than silently lost.
            s_stats.send_failed++;
            return false;
        }
    }

    s_stats.enqueued++;
    UBaseType_t depth = uxQueueMessagesWaiting(s_queue);
    if (depth > s_stats.depth_high_water) {
        s_stats.depth_high_water = (uint16_t)depth;
    }
    return true;
}

Stats stats() {
    // Volatile -> non-volatile copy. Each uint32 read is atomic on
    // Xtensa, so the snapshot is consistent per-field even if the
    // drainer updates concurrently.
    Stats out;
    out.enqueued         = s_stats.enqueued;
    out.dropped_full     = s_stats.dropped_full;
    out.dropped_oversize = s_stats.dropped_oversize;
    out.dropped_unreg    = s_stats.dropped_unreg;
    out.sent_ok          = s_stats.sent_ok;
    out.send_failed      = s_stats.send_failed;
    out.last_drain_ms    = s_stats.last_drain_ms;
    out.depth_high_water = s_stats.depth_high_water;
    return out;
}

uint32_t lastDrainMillis() {
    return s_stats.last_drain_ms;
}

} // namespace udp_tx
