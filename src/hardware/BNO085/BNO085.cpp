#include "BNO085.h"
#include "../../core/globals.h"
#include "../../utils/log.h"
#include "../i2c_manager.h"

BNO085 bno08x;

BNO085::BNO085() : _dataReady(false) {
}

bool BNO085::begin(TwoWire *wirePort) {
    // Initialize the BNO085 sensor
    if (!_bno.begin_I2C(BNO085_I2C_ADDR, wirePort)) {
        error("Failed to find BNO08x sensor");
        return false;
    }
    
    debug("BNO08x sensor found");
    
    // Display sensor information
    for (int n = 0; n < _bno.prodIds.numEntries; n++) {
        debugf("Part %d: Version %d.%d.%d Build %d",
            _bno.prodIds.entry[n].swPartNumber,
            _bno.prodIds.entry[n].swVersionMajor,
            _bno.prodIds.entry[n].swVersionMinor,
            _bno.prodIds.entry[n].swVersionPatch,
            _bno.prodIds.entry[n].swBuildNumber);
    }
    
    // Set up the sensor reports
    if (!setReports()) {
        error("Failed to set BNO08x reports");
        return false;
    }
    
    return true;
}

bool BNO085::setReports() {
    // Enable rotation vector reports - this is the one we want for heading/roll/pitch
    if (!_bno.enableReport(SH2_ROTATION_VECTOR)) {
        error("Could not enable rotation vector report");
        return false;
    }
    
    return true;
}

float BNO085::getHeading() {
    if (!_dataReady) {
        update();
    }
    
    if (_sensorValue.sensorId == SH2_ROTATION_VECTOR) {
        float yaw = getYaw(&_sensorValue.un.rotationVector);
        // Convert to degrees and adjust from [-180;180] to [0;360]
        yaw = -yaw * CONST_180_DIVIDED_BY_PI; // Invert to get clockwise heading
        
        if (yaw < 0 && yaw >= -180) {
            yaw += 360.0f;
        }
        
        return yaw;
    }
    
    return 0.0f;
}

float BNO085::getRoll() {
    if (!_dataReady) {
        update();
    }
    
    if (_sensorValue.sensorId == SH2_ROTATION_VECTOR) {
        float roll = getRoll(&_sensorValue.un.rotationVector);
        return roll * CONST_180_DIVIDED_BY_PI;
    }
    
    return 0.0f;
}

float BNO085::getPitch() {
    if (!_dataReady) {
        update();
    }
    
    if (_sensorValue.sensorId == SH2_ROTATION_VECTOR) {
        float pitch = getPitch(&_sensorValue.un.rotationVector);
        return pitch * CONST_180_DIVIDED_BY_PI;
    }
    
    return 0.0f;
}

void BNO085::update() {
    _dataReady = _bno.getSensorEvent(&_sensorValue);
    
    if (_bno.wasReset()) {
        debug("BNO08x was reset, re-configuring reports");
        setReports();
    }
}

bool BNO085::dataAvailable() {
    _dataReady = _bno.getSensorEvent(&_sensorValue);
    return _dataReady;
}

bool BNO085::wasReset() {
    return _bno.wasReset();
}

void BNO085::delay(uint32_t ms) {
    ::delay(ms);
}

float BNO085::getYaw(sh2_RotationVectorWAcc_t *rv) {
    float dqw = rv->real;
    float dqx = rv->i;
    float dqy = rv->j;
    float dqz = rv->k;

    float norm = sqrt(dqw * dqw + dqx * dqx + dqy * dqy + dqz * dqz);
    dqw = dqw / norm;
    dqx = dqx / norm;
    dqy = dqy / norm;
    dqz = dqz / norm;

    float ysqr = dqy * dqy;

    // yaw (z-axis rotation)
    float t3 = +2.0 * (dqw * dqz + dqx * dqy);
    float t4 = +1.0 - 2.0 * (ysqr + dqz * dqz);
    float yaw = atan2(t3, t4);

    return yaw;
}

float BNO085::getRoll(sh2_RotationVectorWAcc_t *rv) {
    float dqw = rv->real;
    float dqx = rv->i;
    float dqy = rv->j;
    float dqz = rv->k;

    float norm = sqrt(dqw * dqw + dqx * dqx + dqy * dqy + dqz * dqz);
    dqw = dqw / norm;
    dqx = dqx / norm;
    dqy = dqy / norm;
    dqz = dqz / norm;

    float ysqr = dqy * dqy;

    // roll (x-axis rotation)
    float t0 = +2.0 * (dqw * dqx + dqy * dqz);
    float t1 = +1.0 - 2.0 * (dqx * dqx + ysqr);
    float roll = atan2(t0, t1);

    return roll;
}

float BNO085::getPitch(sh2_RotationVectorWAcc_t *rv) {
    float dqw = rv->real;
    float dqx = rv->i;
    float dqy = rv->j;
    float dqz = rv->k;

    float norm = sqrt(dqw * dqw + dqx * dqx + dqy * dqy + dqz * dqz);
    dqw = dqw / norm;
    dqx = dqx / norm;
    dqy = dqy / norm;
    dqz = dqz / norm;

    float ysqr = dqy * dqy;

    // pitch (y-axis rotation)
    float t2 = +2.0 * (dqw * dqy - dqz * dqx);
    t2 = t2 > 1.0 ? 1.0 : t2;
    t2 = t2 < -1.0 ? -1.0 : t2;
    float pitch = asin(t2);

    return pitch;
} 