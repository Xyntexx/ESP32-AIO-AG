#include "bicycle_sim.h"

#if SIMULATOR

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "autosteer/imu.h"
#include "autosteer/was.h"
#include "utils/log.h"

namespace sim {

// World-frame state. heading uses compass convention: 0 = north, +90 = east,
// increasing clockwise. lat/lon in decimal degrees. Speed in m/s.
static double  s_lat        = SIM_INIT_LAT;
static double  s_lon        = SIM_INIT_LON;
static float   s_headingRad = 0.0f;
static float   s_speedMps   = SIM_SPEED_MPS;
static const float s_wheelbase = SIM_WHEELBASE_M;

static bool (*s_udpSend)(const uint8_t*, size_t) = nullptr;

static constexpr float DEG2RAD = 0.0174532925f;
static constexpr float RAD2DEG = 57.2957795f;
static constexpr double METERS_PER_DEG_LAT = 111320.0;

static float headingFn() {
    float deg = s_headingRad * RAD2DEG;
    while (deg < 0.0f)   deg += 360.0f;
    while (deg >= 360.0f) deg -= 360.0f;
    return deg;
}

static float rollFn() { return 0.0f; }

bool init() {
    s_lat        = SIM_INIT_LAT;
    s_lon        = SIM_INIT_LON;
    s_headingRad = 0.0f;
    s_speedMps   = SIM_SPEED_MPS;

    imu::IMUInterface ifc;
    ifc.heading = headingFn;
    ifc.roll    = rollFn;
    imu::init(ifc);

    infof("SIM: bicycle model active (L=%.2fm, v=%.2fm/s, lat0=%.5f lon0=%.5f)",
          s_wheelbase, s_speedMps, s_lat, s_lon);
    return true;
}

void tick() {
    const float dt = SIM_TICK_MS / 1000.0f;

    // WAS gives the (calibrated) steering wheel angle in degrees. Convert to
    // radians for the bicycle model. Clamp so a runaway WAS reading can't
    // blow up tan(delta).
    float deltaDeg = was::get_steering_angle();
    if (deltaDeg >  60.0f) deltaDeg =  60.0f;
    if (deltaDeg < -60.0f) deltaDeg = -60.0f;
    const float deltaRad = deltaDeg * DEG2RAD;

    const float v = s_speedMps;
    const float L = s_wheelbase;

    // Bicycle model with compass-convention heading:
    //   north component  -> dx = v cos(psi)
    //   east  component  -> dy = v sin(psi)
    //   d psi/dt = (v / L) * tan(delta)
    const float dHeading = (v / L) * tanf(deltaRad) * dt;
    s_headingRad += dHeading;

    const float dxNorth = v * cosf(s_headingRad) * dt;
    const float dyEast  = v * sinf(s_headingRad) * dt;

    s_lat += dxNorth / METERS_PER_DEG_LAT;
    const double cosLat = cos(s_lat * DEG2RAD);
    const double mPerDegLon = METERS_PER_DEG_LAT * (cosLat > 1e-6 ? cosLat : 1e-6);
    s_lon += dyEast / mPerDegLon;
}

void setUdpSender(bool (*send)(const uint8_t*, size_t)) {
    s_udpSend = send;
}

float getHeadingDeg() { return headingFn(); }
float getRollDeg()    { return 0.0f; }
double getLat()       { return s_lat; }
double getLon()       { return s_lon; }
float getSpeedMps()   { return s_speedMps; }

// NMEA helpers --------------------------------------------------------------

static uint8_t nmeaChecksum(const char* s, size_t len) {
    uint8_t cs = 0;
    for (size_t i = 0; i < len; i++) cs ^= (uint8_t)s[i];
    return cs;
}

// Format degrees as DDMM.MMMMM (latitude) or DDDMM.MMMMM (longitude). Sign is
// returned through hemiOut so the caller can append N/S or E/W.
static void formatLatLon(double deg, bool isLon, char* buf, size_t bufLen, char* hemiOut) {
    char hemi;
    if (isLon) {
        hemi = (deg >= 0) ? 'E' : 'W';
    } else {
        hemi = (deg >= 0) ? 'N' : 'S';
    }
    *hemiOut = hemi;
    double abs_deg = (deg >= 0) ? deg : -deg;
    int    whole_deg = (int)abs_deg;
    double minutes = (abs_deg - whole_deg) * 60.0;
    if (isLon) {
        snprintf(buf, bufLen, "%03d%08.5f", whole_deg, minutes);
    } else {
        snprintf(buf, bufLen, "%02d%08.5f", whole_deg, minutes);
    }
}

// Synthesize a UTC time string from millis() since boot. AOG mainly cares
// that the time is monotonically increasing and parses, not that it's right.
static void formatUtcTime(char* buf, size_t bufLen) {
    uint32_t total_ms = millis();
    uint32_t total_s  = total_ms / 1000;
    uint32_t hh = (total_s / 3600) % 24;
    uint32_t mm = (total_s / 60) % 60;
    uint32_t ss = total_s % 60;
    uint32_t cs = (total_ms % 1000) / 10; // hundredths
    snprintf(buf, bufLen, "%02u%02u%02u.%02u", hh, mm, ss, cs);
}

static void formatUtcDate(char* buf, size_t bufLen) {
    // Fixed valid-looking date - day/month/year mod 100. AOG just needs a
    // parseable RMC.
    snprintf(buf, bufLen, "010125");
}

static bool sendSentence(char* body, size_t bodyLen) {
    if (!s_udpSend) return false;
    uint8_t cs = nmeaChecksum(body, bodyLen);
    char full[160];
    int n = snprintf(full, sizeof(full), "$%.*s*%02X\r\n", (int)bodyLen, body, cs);
    if (n <= 0 || n >= (int)sizeof(full)) return false;
    return s_udpSend((const uint8_t*)full, (size_t)n);
}

void emitNMEA() {
    if (!s_udpSend) return;

    char timeBuf[16];
    formatUtcTime(timeBuf, sizeof(timeBuf));
    char dateBuf[8];
    formatUtcDate(dateBuf, sizeof(dateBuf));

    char latBuf[16], lonBuf[20];
    char nsHemi, ewHemi;
    formatLatLon(s_lat, false, latBuf, sizeof(latBuf), &nsHemi);
    formatLatLon(s_lon, true,  lonBuf, sizeof(lonBuf), &ewHemi);

    float headingDeg = getHeadingDeg();
    float speedKnots = s_speedMps * 1.94384f;
    float speedKmh   = s_speedMps * 3.6f;

    char body[160];
    int n;

    // GGA: time, lat, lon, fix quality=4 (RTK fixed), sats=14, hdop=0.6,
    // alt, geoid separation. AgOpenGPS gates engagement on a "good" fix and
    // typically requires quality >= 4 (RTK fixed) or 5 (RTK float).
    n = snprintf(body, sizeof(body),
                 "GNGGA,%s,%s,%c,%s,%c,4,14,0.6,100.0,M,46.9,M,1.0,0000",
                 timeBuf, latBuf, nsHemi, lonBuf, ewHemi);
    if (n > 0 && n < (int)sizeof(body)) sendSentence(body, n);

    // RMC: time, status=A (valid), lat, lon, speed (knots), course, date,
    // mag var (empty), mode=R (RTK). RMC mode-indicator field is the last
    // value before the checksum and corresponds to the GGA fix quality:
    //   A=autonomous, D=DGPS, F=RTK float, R=RTK fixed, E=estimated, N=invalid.
    n = snprintf(body, sizeof(body),
                 "GNRMC,%s,A,%s,%c,%s,%c,%.2f,%.2f,%s,,,R",
                 timeBuf, latBuf, nsHemi, lonBuf, ewHemi,
                 (double)speedKnots, (double)headingDeg, dateBuf);
    if (n > 0 && n < (int)sizeof(body)) sendSentence(body, n);

    // VTG: course true, course magnetic, speed knots, speed km/h, mode=R
    // (RTK fixed) to match GGA quality=4.
    n = snprintf(body, sizeof(body),
                 "GNVTG,%.2f,T,%.2f,M,%.2f,N,%.2f,K,R",
                 (double)headingDeg, (double)headingDeg,
                 (double)speedKnots, (double)speedKmh);
    if (n > 0 && n < (int)sizeof(body)) sendSentence(body, n);
}

} // namespace sim

#endif // SIMULATOR
