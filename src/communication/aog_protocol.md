# AgOpenGPS Protocol Documentation

**Verified against AgOpenGPS source code v5.x (2025-10-28)**

## Network Architecture

### AgOpenGPS Components
- **AgOpenGPS** (main application): Sends data from port 9999
- **AgIO** (communication hub): Broadcasts to modules on port 8888
- **Modules** (Steer, Machine, IMU, GPS): Listen on port 8888, respond to queries

### Port Configuration
- **Module Listening Port**: 8888 (all modules listen here for broadcasts from AgIO)
- **AgOpenGPS Send Port**: 9999 (sends to AgIO)
- **AgIO Loopback Port**: 17777 (internal communication)

### IP Addressing
- **Default Subnet**: 192.168.5.x (configurable via subnet change message)
- **Module IPs**: Typically 192.168.5.12x where x = module ID
  - Steer: 192.168.5.126 (suggested)
  - Machine: 192.168.5.123 (suggested)
  - IMU: 192.168.5.121 (suggested)
  - GPS: 192.168.5.124 (suggested)
- **Broadcast**: 192.168.5.255:8888 (AgIO broadcasts to all modules)

### Message Flow
1. AgOpenGPS sends PGN messages to AgIO (port 9999)
2. AgIO broadcasts messages to all modules (port 8888, subnet broadcast)
3. Modules receive, process, and reply to AgIO (port 8888)
4. AgIO forwards replies back to AgOpenGPS

## Protocol Structure

### Message Format
- **Endianness**: Little-endian (LSB first, MSB second)
- **Multi-byte values**: For 16-bit values (uint16_t), the low byte is sent first, then the high byte
- **Message Structure**: `[Header][Source][PGN][Length][Data...][CRC]`
  - Header: `0x80, 0x81` (fixed, 2 bytes)
  - Source: Module ID (1 byte, typically 0x7F for AgOpenGPS)
  - PGN: Parameter Group Number (1 byte)
  - Length: Data length in bytes (1 byte)
  - Data: Variable length payload
  - CRC: Simple byte sum checksum (1 byte)
- **Checksum**: Sum of all bytes from position 2 (after 0x80, 0x81) to length-2, result cast to byte

## Steer Module

- **Module ID**: 126 (0x7E)
- **Suggested IP**: 192.168.5.126
- **Listen Port**: 8888 (receives broadcasts from AgIO)
- **Send Port**: 8888 (sends replies to AgIO)

| PGN Name         | Src | Dec | PGN | Dec | Len | Byte 5                        | Byte 6               | Byte 7                | Byte 8                   | Byte 9                 | Byte 10                   | Byte 11           | Byte 12               | Byte 13 |
|------------------|-----|-----|-----|-----|-----|-------------------------------|----------------------|-----------------------|--------------------------|------------------------|---------------------------|-------------------|-----------------------|---------|
| Steer Data       | 7F  | 127 | FE  | 254 | 8   | Speed (LSB) [km/h]            | Speed (MSB)          | Status                | steerAngle (LSB) [deg]   | steerAngle (MSB)      | xte (cross-track error)   | SC1to8            | SC9to16               | CRC     |
| Steer Settings   | 7F  | 127 | FC  | 252 | 8   | gainP                         | highPWM              | lowPWM                | minPWM                   | countsPerDeg          | steerOffset (LSB) [deg]   | steerOffset (MSB) | ackermanFix           | CRC     |
| Steer Config     | 7F  | 127 | FB  | 251 | 8   | set0                          | pulseCount           | minSpeed [km/h]       | sett1                    | reserved              | reserved                  | reserved          | reserved              | CRC     |
| From AutoSteer   | 7E  | 126 | FD  | 253 | 8   | ActualSteerAngle (LSB) [deg]  | ActualSteerAngle (MSB) | IMU Heading (LSB) [deg] | IMU Heading (MSB)      | IMU Roll (LSB) [deg]  | IMU Roll (MSB)            | Switch            | PWMDisplay            | CRC     |
| From Autosteer 2 | 7F  | 127 | FA  | 250 | 8   | Sensor Value                  | reserved             | reserved              | reserved                 | reserved              | reserved                  | reserved          | reserved              | CRC     |

## Machine Module

- **Module ID**: 123 (0x7B)
- **Suggested IP**: 192.168.5.123
- **Listen Port**: 8888 (receives broadcasts from AgIO)
- **Send Port**: 8888 (sends replies to AgIO)

| PGN Name       | Src | Dec | PGN | Dec | Len | Byte 5              | Byte 6              | Byte 7              | Byte 8            | Byte 9              | Byte 10           | Byte 11           | Byte 12             | Byte 13 |
|----------------|-----|-----|-----|-----|-----|---------------------|---------------------|---------------------|-------------------|---------------------|-------------------|-------------------|---------------------|---------|
| Machine Data   | 7F  | 127 | EF  | 239 | 8   | uturn               | speed*10 [km/h]     | hydLift             | Tram              | GeoStop             | reserved          | SC1to8            | SC9to16             | CRC     |
| Machine Config | 7F  | 127 | EE  | 238 | 8   | raiseTime           | lowerTime           | hydEnable           | set0              | User1               | User2             | User3             | User4               | CRC     |
| From Machine   | 7B  | 123 | ED  | 237 | 8   | Status1             | Status2             | Status3             | Status4           | Status5             | Status6           | Status7           | Status8             | CRC     |

## IMU Module

- **Module ID**: 121 (0x79)
- **Suggested IP**: 192.168.5.121
- **Listen Port**: 8888 (receives broadcasts from AgIO)
- **Send Port**: 8888 (sends replies to AgIO)

| PGN Name | Src | Dec | PGN | Dec | Len | Byte 5                 | Byte 6        | Byte 7              | Byte 8     | Byte 9              | Byte 10    | Byte 11 | Byte 12 | Byte 13 |
|----------|-----|-----|-----|-----|-----|------------------------|---------------|---------------------|------------|---------------------|------------|---------|---------|---------|
| From IMU | 79  | 121 | D3  | 211 | 8   | Heading (LSB) [deg]    | Heading (MSB) | Roll (LSB) [deg]    | Roll (MSB) | Gyro (LSB)          | Gyro (MSB) | 0       | 0       | CRC     |

## GPS (Main Antenna)

- **Module ID**: 124 (0x7C)
- **Suggested IP**: 192.168.5.124
- **Listen Port**: 8888 (receives broadcasts from AgIO)
- **Send Port**: 8888 (sends NMEA stream to AgIO)

| PGN Name      | Src | Dec | PGN | Dec | Len | Data Format                                                    |
|---------------|-----|-----|-----|-----|-----|----------------------------------------------------------------|
| Main Antenna  | 7C  | 124 | D6  | 214 |     | NMEA GPS data stream (not byte-structured like other messages) |

## GPS (Tool/Heading Antenna)

- **Module ID**: 125 (0x7D)
- **Suggested IP**: 192.168.5.125
- **Listen Port**: 8888 (receives broadcasts from AgIO)
- **Send Port**: 8888 (sends NMEA stream to AgIO)

| PGN Name      | Src | Dec | PGN | Dec | Len | Data Format                                                    |
|---------------|-----|-----|-----|-----|-----|----------------------------------------------------------------|
| Tool Antenna  | 7D  | 125 | D7  | 215 |     | NMEA GPS data stream (not byte-structured like other messages) |

## Hello Message Protocol

| PGN Name            | Src | Dec | PGN | Dec | Len | Byte 5              | Byte 6            | Byte 7             | Byte 8             | Byte 9               | Byte 10 |
|---------------------|-----|-----|-----|-----|-----|---------------------|-------------------|--------------------|--------------------|----------------------|---------|
| Hello to Module     | 7F  | 127 | C8  | 200 | 3   | Module ID           | 0                 | 0                  | CRC                |                      |         | 
| Hello Reply Steer   | 7E  | 126 | 7E  | 126 | 5   | AngleLo [deg]       | AngleHi           | CountsLo           | CountsHi           | Switchbyte           | CRC     |
| Hello Reply Machine | 7B  | 123 | 7B  | 123 | 5   | relayLo             | relayHi           | reserved           | reserved           | reserved             | CRC     |

## Subnet Configuration

| PGN Name             | Src | Dec | PGN | Dec | Len | Byte 5          | Byte 6          | Byte 7              | Byte 8           | Byte 9                 | Byte 10 |
|----------------------|-----|-----|-----|-----|-----|-----------------|-----------------|---------------------|------------------|------------------------|---------|
| Subnet Change        | 7F  | 127 | C9  | 201 | 5   | 201             | 201             | IP_One              | IP_Two           | IP_Three               | CRC     |
| Scan Request         | 7B  | 123 | CA  | 202 | 3   | 202             | 202             | 5                   | CRC              |                        |         |
| Subnet Reply Steer   | 7E  | 126 | CB  | 203 | 7   | ipOne           | ipTwo           | ipThree             | 126              | SrcOne-Three           | CRC     |
| Subnet Reply Machine | 7B  | 123 | CB  | 203 | 7   | ipOne           | ipTwo           | ipThree             | 123              | SrcOne-Three           | CRC     |

