//
// Created by MarkusNuuja on 06/05/2025.
//

#include "hardware.h"
#include "imu/bno08x_imu.h"
#include "was/ads1115_was.h"


namespace hw{

bool init(){
    BNO08XIMU::init(); // Init BNO first since they use the same i2c.
    ADS1115WAS::init();
}


}