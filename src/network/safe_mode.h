#ifndef SAFE_MODE_H
#define SAFE_MODE_H

namespace safe_mode {

// Run the safe-mode probe. Call once during setup(), after Ethernet + UDP
// logging + OTA are up but BEFORE hw::init() / create_tasks(). Listens
// for a magic UDP packet ("HALT-AIO-AG") on port 7779 for a short window
// (~1.5s); if seen, engages safe mode for the rest of this boot.
//
// Safe mode skips hardware init, task creation, and AOG UDP listeners -
// the device sits in the OTA-only handler loop forever. Recovery from a
// boot loop is: watch the UDP log to confirm the device is in fact
// rebooting, blast HALT-AIO-AG during the next boot's 1.5s window, then
// OTA-flash a fix. Next reboot proceeds normally (no HALT seen).
//
// We deliberately do NOT auto-engage on crash-flavored reset reasons -
// a transient field event (brownout, cosmic-ray panic) shouldn't take
// the device out of service. The 1.5s window is short enough to feel
// like normal boot but long enough for a user-driven escape hatch.
void checkAtBoot();

// True if checkAtBoot() decided to engage safe mode.
bool active();

// Periodic "still alive" log emitted from the safe-mode loop. Self
// rate-limits so it can be called every loop iteration.
void tickHeartbeat();

// Bring up the always-on runtime control listener on UDP 7779. After
// boot, accepts 'REBOOT-AIO-AG' to esp_restart() the device. Call once
// from setup() after Ethernet is up. Independent of the boot-window
// listener inside checkAtBoot().
bool initRuntimeListener();

} // namespace safe_mode

#endif // SAFE_MODE_H
