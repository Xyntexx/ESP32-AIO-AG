//
// Created by Markus on 5.5.2025.
//

#ifndef AUTOSTEER_CONFIG_H
#define AUTOSTEER_CONFIG_H

#define LOW_HIGH_DEGREES 3.0
// Watchdog timeout in ms. AOG SteerData arrives at ~10Hz (~100ms cadence);
// 200ms was too tight - normal UDP jitter would push one packet past that
// and disengage autosteer (resetting NONE-mode arming in the process,
// requiring AOG to toggle status off->on again). 500ms tolerates ~5
// missed packets - generous against jitter, still short enough to catch
// a real AOG outage promptly.
#define WATCHDOG_TIMEOUT 500


#endif //AUTOSTEER_CONFIG_H
