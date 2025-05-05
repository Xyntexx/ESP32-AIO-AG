# AgOpenGPS Protocol Documentation

## Endianness and Data Format
- **Endianness**: Little-endian (LSB first, MSB second)
- **Multi-byte values**: For 16-bit values (uint16_t), the low byte is sent first, then the high byte
- **Message Structure**: All messages begin with a header (80,81,7F for newer protocol versions)
- **Checksum**: Last byte is a simple sum of all preceding bytes in the message after the header

## Steer Module

IP = 192.168.5.126

Hello = 126

Port = 5126

00 00 56 00 00 7E

| PGN Name         | Src | Dec | PGN | Dec | Len | Byte 5                        | Byte 6               | Byte 7                | Byte 8                   | Byte 9                 | Byte 10                   | Byte 11           | Byte 12               | Byte 13 |
|------------------|-----|-----|-----|-----|-----|-------------------------------|----------------------|-----------------------|--------------------------|------------------------|---------------------------|-------------------|-----------------------|---------|
| Steer Data       | 7F  | 127 | FE  | 254 | 8   | Speed (LSB) [km/h]            | Speed (MSB)          | Status                | steerAngle (LSB) [deg]   | steerAngle (MSB)      | xte (cross-track error)   | SC1to8            | SC9to16               | CRC     |
| Steer Settings   | 7F  | 127 | FC  | 252 | 8   | gainP                         | highPWM              | lowPWM                | minPWM                   | countsPerDeg          | steerOffset (LSB) [deg]   | steerOffset (MSB) | ackermanFix           | CRC     |
| Steer Config     | 7F  | 127 | FB  | 251 | 8   | set0                          | pulseCount           | minSpeed [km/h]       | sett1                    | reserved              | reserved                  | reserved          | reserved              | CRC     |
| From AutoSteer   | 7E  | 126 | FD  | 253 | 8   | ActualSteerAngle (LSB) [deg]  | ActualSteerAngle (MSB) | IMU Heading (LSB) [deg] | IMU Heading (MSB)      | IMU Roll (LSB) [deg]  | IMU Roll (MSB)            | Switch            | PWMDisplay            | CRC     |
| From Autosteer 2 | 7F  | 127 | FA  | 250 | 8   | Sensor Value                  | reserved             | reserved              | reserved                 | reserved              | reserved                  | reserved          | reserved              | CRC     |

## Machine Module

IP = 192.168.5.123

Hello = 123

Port = 5123

00 00 56 00 00 7B

| PGN Name       | Src | Dec | PGN | Dec | Len | Byte 5              | Byte 6              | Byte 7              | Byte 8            | Byte 9              | Byte 10           | Byte 11           | Byte 12             | Byte 13 |
|----------------|-----|-----|-----|-----|-----|---------------------|---------------------|---------------------|-------------------|---------------------|-------------------|-------------------|---------------------|---------|
| Machine Data   | 7F  | 127 | EF  | 239 | 8   | uturn               | speed*10 [km/h]     | hydLift             | Tram              | GeoStop             | reserved          | SC1to8            | SC9to16             | CRC     |
| Machine Config | 7F  | 127 | EE  | 238 | 8   | raiseTime           | lowerTime           | hydEnable           | set0              | User1               | User2             | User3             | User4               | CRC     |
| From Machine   | 7B  | 123 | ED  | 237 | 8   | Status1             | Status2             | Status3             | Status4           | Status5             | Status6           | Status7           | Status8             | CRC     |

## IMU Module

IP = 192.168.5.121

Hello = 121

Port = 5121

00 00 56 00 00 79

| PGN Name | Src | Dec | PGN | Dec | Len | Byte 5                 | Byte 6        | Byte 7              | Byte 8     | Byte 9              | Byte 10    | Byte 11 | Byte 12 | Byte 13 |
|----------|-----|-----|-----|-----|-----|------------------------|---------------|---------------------|------------|---------------------|------------|---------|---------|---------|
| From IMU | 79  | 121 | D3  | 211 | 8   | Heading (LSB) [deg]    | Heading (MSB) | Roll (LSB) [deg]    | Roll (MSB) | Gyro (LSB)          | Gyro (MSB) | 0       | 0       | CRC     |

## GPS

IP = 192.168.5.124

Hello = na

Port = 5124

00 00 56 00 00 7C

| PGN Name      | Src | Dec | PGN | Dec | Len | Data Format                                                    |
|---------------|-----|-----|-----|-----|-----|----------------------------------------------------------------|
| Main Antenna  | 7C  | 124 | D6  | 214 |     | NMEA GPS data stream (not byte-structured like other messages) |

## Tool GPS

IP = 192.168.5.125

Hello = 125

Port = 10000

00 00 56 00 00 7D

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

