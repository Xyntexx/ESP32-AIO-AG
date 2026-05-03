//
// Created by Markus on 5.5.2025.
//

#ifndef AUTOSTEER_CONFIG_H
#define AUTOSTEER_CONFIG_H

#define LOW_HIGH_DEGREES 3.0
// Watchdog timeout in ms. AOG SteerData arrives at ~10Hz (~100ms cadence).
// 500ms (5 missed packets) was still tripping under real-world AOG jitter -
// observed an 800ms gap caused by AOG's own scheduling pause that triggered
// a spontaneous disengage even though the underlying network was healthy.
// 1500ms (15 missed packets) leaves plenty of room for AOG to resume after
// a stall while still catching a genuine multi-second outage. The safety
// task's 100ms autosteer-stale watchdog still guards the control loop, so
// loosening this only affects guidance-source freshness, not motor safety.
#define WATCHDOG_TIMEOUT 1500


#endif //AUTOSTEER_CONFIG_H
