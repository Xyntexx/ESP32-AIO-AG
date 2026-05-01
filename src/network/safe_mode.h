#ifndef SAFE_MODE_H
#define SAFE_MODE_H

namespace safe_mode {

// Run the safe-mode probe. Call once during setup(), after Ethernet + UDP
// logging + OTA are up but BEFORE hw::init() / create_tasks(). The probe
// engages safe mode (suspending normal boot) when either of these is true:
//
//   1) The previous reset reason looks like a crash (panic, task/int
//      watchdog, brownout, generic WDT). Reboots from POWERON / EXT pin /
//      software reset (e.g. a clean OTA finalize) are treated as healthy.
//
//   2) A magic UDP packet ("HALT-AIO-AG") arrives on port 7777 within a
//      ~1.5s window. Lets the user force safe mode at will - useful when
//      the firmware wedges without producing a recognized reset reason
//      (e.g. an I2C mutex starve that times out a task watchdog the
//      firmware can't print before the reset).
//
// In safe mode the device stays in the OTA-only handler loop forever; no
// hardware init, no tasks, no AOG UDP listeners. Recovery is: flash a fix
// via OTA, reboot. The next reset reason will be ESP_RST_SW (clean) and
// the firmware will resume normal boot.
void checkAtBoot();

// True if checkAtBoot() decided to engage safe mode.
bool active();

// Periodic "still alive" log emitted from the safe-mode loop. Self
// rate-limits so it can be called every loop iteration.
void tickHeartbeat();

} // namespace safe_mode

#endif // SAFE_MODE_H
