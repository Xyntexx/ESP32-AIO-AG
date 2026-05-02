#ifndef SAFETY_H
#define SAFETY_H

#include <stdint.h>

namespace safety {

// High-priority motor-safety task. Runs at SAFETY_TASK_PRIORITY (above
// every task that could legitimately drive the motor) and continuously
// asserts that if any of the safety conditions are not met, the motor
// is stopped - independent of whether the autosteer task or the network
// stack is healthy.
//
// What it monitors (read directly from existing module state, no IPC):
//   1. guidancePacketValid()  - AOG steer-data RX freshness
//   2. autosteer::isEngaged() - composite hw + sw enable
//   3. KeyaMotor::isHealthy() (KEYA_MOTOR builds only) - heartbeat alive
//   4. autosteer-task heartbeat (noteAutosteerTick) - control loop alive
//
// What it does: motor::stopMotor() when any of those drops. Never
// engages the motor. Never sends UDP. Never touches I2C/SPI. Never
// blocks on a queue.
bool init();

// Called from autosteer::handler() on every tick. Bumps an atomic
// counter the safety task watches for staleness; if the counter hasn't
// moved within SAFETY_AUTOSTEER_STALE_MS the safety task force-stops.
void noteAutosteerTick();

// Telemetry. Number of safety stops since boot (any reason) - useful
// for knowing whether the safety task ever fired in a session.
uint32_t stopCount();

} // namespace safety

#endif // SAFETY_H
