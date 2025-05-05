#include "imu.h"

#include "core/globals.h"
#include "settings/settings.h"
#include "utils/log.h"

namespace imu {
float heading = 0.0;
float roll    = 0.0;
float pitch   = 0.0;

[[noreturn]] void imu_task_loop(void *pv_parameters) {
    for (;;) {
        switch (Set.imuType) {
            case IMUType::BNO08X: // BNO08X
                heading = bno08x.getHeading();
                roll  = bno08x.getRoll();
                pitch = bno08x.getPitch();
                bno08x.delay();
                break;
            case IMUType::BNO055: // BNO055
                warning("BNO055 not implemented");
                heading = 0.0;
                roll    = 0.0;
                pitch   = 0.0;
                delay(default_delay);
                break;
            case IMUType::CMPS14: // CMPS14
                warning("CMPS14 not implemented");
                heading = 0.0;
                roll    = 0.0;
                pitch   = 0.0;
                delay(default_delay);
                break;
            case IMUType::NONE: // No IMU
                warning("No IMU selected");
                heading = 0.0;
                roll    = 0.0;
                pitch   = 0.0;
                delay(default_delay);
                break;
            default:
                warning("Unknown IMU type");
                heading = 0.0;
                roll    = 0.0;
                pitch   = 0.0;
                delay(default_delay);
                break;
        }
    }
}

float get_heading() {
    return heading;
}

float get_roll() {
    return roll;
}

float get_pitch() {
    return pitch;
}

bool init() {
    xTaskCreate(imu_task_loop, "imu_task", 4096, nullptr, IMU_TASK_PRIORITY, nullptr);
    return true;
}
}

// if (Set.IMUType > 0) {
// 	//imu heading16 --- * 10 in degrees,  V17= *16
//
// 	switch (Set.IMUType) {
// 		case 1:// BNO055 heading16
// 			heading16 = (int)BNO.euler.head;  //heading16 in degrees * 16 from BNO
// 			heading = float(heading16) / 16;
// 			temInt = int(heading * 10);
// 			steerToAOG[8] = temInt >> 8;
// 			steerToAOG[7] = byte(temInt);
// 			break;
//
// 		case 2://CMPS14
// 			Wire.beginTransmission(Set.CMPS14_ADDRESS);
// 			Wire.write(0x02);
// 			Wire.endTransmission();
//
// 			Wire.requestFrom(Set.CMPS14_ADDRESS, 2);
// 			while (Wire.available() < 2);
//
// 			//the heading16 x10
// 			steerToAOG[8] = Wire.read();
// 			steerToAOG[7] = Wire.read();
// 			//convert to float for WebIO
// 			heading16 = steerToAOG[8];
// 			heading16 <<= 8;
// 			heading16 += steerToAOG[7];
// 			heading = float(heading16) * 0.1;
//
// 			Wire.beginTransmission(Set.CMPS14_ADDRESS);
// 			Wire.write(0x1C);
// 			Wire.endTransmission();
//
// 			Wire.requestFrom(Set.CMPS14_ADDRESS, 2);
// 			while (Wire.available() < 2);
//
// 			//the roll x10
// 			steerToAOG[10] = Wire.read();
// 			steerToAOG[9] = Wire.read();
// 			//convert to float for WebIO
// 			roll16 = steerToAOG[10];
// 			roll16 <<= 8;
// 			roll16 += steerToAOG[9];
// 			roll = float(roll16) * 0.1;
// 			//if (bitRead(roll16, 15)) { roll -= 6554; }
// 			if (Set.InvertRoll) { roll *= -1; }//affects only number shown in Webinterface
// 			break;
//
// 		case 3:
// 			if (bno08x.dataAvailable() == false) { vTaskDelay(2); }//wait 2ms if no new data from BNO
// 			if (bno08x.dataAvailable() == true)
// 			{
// 				bno08xHeading = (bno08x.getYaw()) * 180.0 / PI; // Convert yaw / heading16 to degrees
// 				bno08xHeading = -bno08xHeading; //BNO085 counter clockwise data to clockwise data
//
// 				if (bno08xHeading < 0 && bno08xHeading >= -180) //Scale BNO085 yaw from [-180�;180�] to [0;360�]
// 				{
// 					bno08xHeading = bno08xHeading + 360;
// 				}
//
// 				bno08xRoll = (bno08x.getRoll()) * 180.0 / PI; //Convert roll to degrees
// 				bno08xPitch = (bno08x.getPitch()) * 180.0 / PI; // Convert pitch to degrees
//
// 				bno08xHeading10x = (int)(bno08xHeading * 10);
// 				bno08xRoll10x = (int)(bno08xRoll * 10);
// 				heading = bno08xHeading;
// 				roll = float(bno08xRoll);
//
// 				//the heading16 x10
// 				steerToAOG[8] = (byte)bno08xHeading10x;
// 				steerToAOG[7] = bno08xHeading10x >> 8;
//
// 				//the roll x10
// 				steerToAOG[10] = (byte)bno08xRoll10x;
// 				steerToAOG[9] = bno08xRoll10x >> 8;
// 			}
// 			break;
