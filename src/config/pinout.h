// Pin Definitions
// ------------------------------
// Default pin assignments for autosteer hardware

#ifndef PINOUT_H
#define PINOUT_H

#define GPS_RX_PIN 1
#define GPS_TX_PIN 2

#define WORK_BTN_PIN 12
#define STEER_BTN_PIN 13

#define STEER_BTN_ACTIVE_STATE 1
#define WORK_BTN_ACTIVE_STATE 1

#define I2C_SDA_PIN 5
#define I2C_SCL_PIN 4

#define ADC_DREADY_PIN 14

#define MOTOR_ENABLE_PIN 8
#define MOTOR_PWM_PIN 9
#define MOTOR_DIR_PIN 10
#define MOTOR_CURRENT_PIN 11

#define W6100_CS_GPIO 18
#define W6100_RESET_GPIO 7
#define W6100_SCK_GPIO 15
#define W6100_MISO_GPIO 16
#define W6100_MOSI_GPIO 17
#define W6100_INT_GPIO 6

//#define LED1_GPIO 41
//#define LED2_GPIO 42

#endif //PINOUT_H
