//
// Created by Markus on 5.5.2025.
//

#include "inputs.h"

bool getEnable() {

    // switch (Set.SteerSwitchType)
    // {
    //     case 0:
    //         steerEnable = digitalRead(Set.STEERSW_PIN);
    //         steerSwitch = steerEnable;
    //         if (steerEnableOld != steerEnable) {
    //             if (Set.debugmode) {
    //                 if (steerEnable) { Serial.println("Autosteer ON by Switch"); }
    //                 else { Serial.println("Autosteer OFF by Switch"); }
    //             }
    //             steerEnableOld = steerEnable;
    //         }
    //         break;
    //     case 1:
    //         steerEnable = !digitalRead(Set.STEERSW_PIN);
    //         steerSwitch = steerEnable;
    //         if (steerEnableOld != steerEnable) {
    //             if (Set.debugmode) {
    //                 if (steerEnable) { Serial.println("Autosteer ON by Switch"); }
    //                 else { Serial.println("Autosteer OFF by Switch"); }
    //             }
    //             steerEnableOld = steerEnable;
    //         }
    //         break;
    //     case 2:
    //         if (toggleSteerEnable) //may set to true in timed loop (to debounce)
    //         {
    //             steerEnable = !steerEnable;
    //             steerEnableOld = steerEnable;
    //             if (Set.debugmode) { Serial.println("Steer-Break: IRQ occured? Button pressed?"); }
    //             toggleSteerEnable = false;
    //         }
    //         steerSwitch = steerEnable;
    //         break;
    //     case 3:
    //         tempvalue = analogRead(Set.STEERSW_PIN);
    //         if (tempvalue < 800) { steerEnable = false; }
    //         if (tempvalue > 3200) { steerEnable = true; }
    //         if (steerEnableOld != steerEnable) {
    //             if (Set.debugmode) {
    //                 if (steerEnable) { Serial.println("Autosteer ON by Switch"); }
    //                 else { Serial.println("Autosteer OFF by Switch"); }
    //             }
    //             steerEnableOld = steerEnable;
    //         }
    //         steerSwitch = steerEnable;
    //         break;
    return false;
}