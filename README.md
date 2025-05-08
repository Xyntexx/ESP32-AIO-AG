# ESP32 All-in-One AgOpenGPS Project

This project is designed for an ESP32-based custom board for AgOpenGPS applications. It utilizes an ESP32-S3 microcontroller and integrates various sensors and modules for AgOpenGPS guidance and control.

## Prerequisites

### Common for Windows and Linux

*   **Visual Studio Code**: Recommended editor for PlatformIO development. Download from [https://code.visualstudio.com/](https://code.visualstudio.com/)
*   **PlatformIO IDE Extension**: Install this extension from the VS Code marketplace. It's used for building and uploading the firmware.
*   **Git**: For cloning the repository. Download from [https://git-scm.com/downloads](https://git-scm.com/downloads)

### Windows

*   **USB-to-UART Bridge VCP Drivers**: Depending on the USB-to-Serial chip on your ESP32-S3 board (e.g., CP210x, CH340/CH341), you might need to install specific drivers.
    *   CP210x: [Silicon Labs Drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
    *   CH340/CH341: Search for "CH340 CH341 serial driver" for your Windows version.

### Linux (Debian-based like Ubuntu)

*   **Build Essentials**:
    ```bash
    sudo apt update
    sudo apt install build-essential
    ```
*   **Python 3 and pip**: PlatformIO Core requires Python.
    ```bash
    sudo apt install python3 python3-pip
    ```
*   **udev rules for serial port access** (usually not needed for modern kernels, but good to be aware of):
    Sometimes, you might need to add your user to the `dialout` group to access serial ports without `sudo`:
    ```bash
    sudo usermod -a -G dialout $USER
    ```
    You will need to log out and log back in for this change to take effect.

## Setup Instructions

1.  **Clone the Repository**:
    Open a terminal or command prompt and run:
    ```bash
    git clone https://github.com/Xyntexx/ESP32-AIO-AG.git
    cd ESP32-AIO-AG
    ```

2.  **Open Project in VS Code**:
    *   Launch VS Code.
    *   Go to `File > Open Folder...` and select the cloned project directory.
    *   VS Code with the PlatformIO extension should automatically recognize it as a PlatformIO project.

3.  **Install Dependencies**:
    PlatformIO will typically prompt you to install project-specific libraries listed in `platformio.ini` automatically. If not, or to do it manually:
    *   Open the PlatformIO CLI terminal in VS Code (usually an icon on the VS Code status bar or by searching "PlatformIO: New Terminal" in the command palette).
    *   Run:
        ```bash
        pio lib install
        ```
    This will download and install the libraries specified in the `lib_deps` section of your `platformio.ini` file:
    *   `adafruit/Adafruit BNO08x@^1.2.3`
    *   `sparkfun/SparkFun u-blox GNSS Arduino Library @ ^2.2.20`
    *   `https://github.com/Xyntexx/WebServer_ESP32_SC_W6100`

## Building and Uploading the Firmware

1.  **Connect the ESP32 Board**:
    Connect your ESP32-S3 board to your computer via USB. Ensure the correct drivers (mentioned in Prerequisites) are installed.

2.  **Select the Correct Environment** (if multiple are defined in `platformio.ini`):
    Your `platformio.ini` currently defines the `esp32-s3` environment. This should be selected by default.

3.  **Build the Project**:
    *   In VS Code, click the PlatformIO icon in the sidebar.
    *   Under the "PROJECT TASKS" for your environment (`env:esp32-s3`), click on "Build".
    *   Alternatively, open the PlatformIO CLI terminal and run:
        ```bash
        pio run
        ```

4.  **Upload the Firmware**:
    *   Once the build is successful, click on "Upload" under "PROJECT TASKS".
    *   Alternatively, in the PlatformIO CLI terminal, run:
        ```bash
        pio run --target upload
        ```
    *   **Troubleshooting Upload**:
        *   If the upload fails, you might need to put the ESP32-S3 into bootloader mode. This usually involves holding down a "BOOT" or "FLASH" button, pressing and releasing the "RESET" or "EN" button, and then releasing the "BOOT" button. The exact procedure can vary by board.
        *   Ensure the correct `upload_protocol` (currently `esp-builtin`) and `board` (currently `esp32-s3-devkitm-1`) are specified in `platformio.ini`.

5.  **Alternative: Uploading via Web Browser (Chrome)**:
    As an alternative to PlatformIO's upload command, you can use a web-based flasher like [ESP Web Tools](https://www.espboards.dev/tools/program/) (requires a Chromium-based browser like Google Chrome or Microsoft Edge).

    *   **Build the Binaries**: First, ensure you have successfully built the project using PlatformIO (`pio run`). This will generate the necessary binary files in the project's `.pio/build/esp32-s3/` directory. You will typically need:
        *   `firmware.bin`
        *   `bootloader.bin` (e.g., `bootloader_dio_80m.bin`)
        *   `partitions.bin` (e.g., `partitions.bin`)
        *   `boot_app0.bin` (optional, sometimes needed)

    *   **Connect the Board in Bootloader Mode**:
        *   Disconnect the ESP32 board.
        *   Press and hold the "BOOT" (or "FLASH") button on your ESP32 board.
        *   While holding the "BOOT" button, connect the board to your computer via USB.
        *   Release the "BOOT" button. The board should now be in bootloader mode.

    *   **Use the Web Flasher**:
        *   Open [https://www.espboards.dev/tools/program/](https://www.espboards.dev/tools/program/) in your Chrome or Edge browser.
        *   Click "CONNECT". A pop-up will appear asking you to select the serial port your ESP32 is connected to. Select it and click "Connect".
        *   The tool will show memory addresses and fields to add your binary files:
            *   `0x0000`: `bootloader.bin` (Browse to `.pio/build/esp32-s3/bootloader.bin`)
            *   `0x8000`: `partitions.bin` (Browse to `.pio/build/esp32-s3/partitions.bin`)
            *   `0xE000`: `boot_app0.bin` (Browse to `.pio/build/esp32-s3/boot_app0.bin`)
            *   `0x10000`: `firmware.bin` (Browse to `.pio/build/esp32-s3/firmware.bin`)
            *(Note: The exact names for `bootloader.bin` might vary slightly, e.g., `bootloader_qio_80m.bin`. Check your `.pio/build/esp32-s3/` directory for the exact filenames. The memory addresses are typical for ESP32-S3 but verify if your board or partition scheme has specific requirements.)*
        *   Once all necessary files are added, click the "PROGRAM" button.
        *   Wait for the flashing process to complete.
        *   Once done, you can disconnect and reconnect the board, or press the "RESET" button to run the new firmware.

## Configuration

*(Further details about configuring the project via `defines.h`, `pinout.h`, or other settings files would go here. Based on the file structure, key configuration files appear to be in the `src/config/` directory.)*

*   **Pinout**: Review and adjust `src/config/pinout.h` to match your custom board's hardware connections.
*   **Defines**: Check `src/config/defines.h` for any compile-time settings that need to be configured for your specific setup (e.g., sensor addresses, feature flags).

## Usage

*(Instructions on how the system interacts with AgOpenGPS, what data it provides, and how to verify its operation would go here. This would typically involve connecting to AgOpenGPS software via UDP/network and observing sensor data or autosteer behavior.)*

## Project Structure Overview

*   `platformio.ini`: PlatformIO project configuration file (board, framework, libraries).
*   `src/`: Contains the main source code.
    *   `core/main.cpp`: Main application entry point.
    *   `config/`: Configuration files (pinouts, global defines).
    *   `autosteer/`: Code related to autosteering logic, IMU, motor control, etc.
    *   `gps/`: GPS module interface.
    *   `hardware/`: Low-level hardware drivers and interfaces (I2C, specific sensors).
    *   `network/`: Ethernet and UDP communication.
    *   `utils/`: Utility functions like logging.
    *   `tasks.cpp`, `tasks.h`: RTOS task definitions.

## Contributing

*(If you plan to have others contribute, add guidelines here.)*

## License

*(Specify the license for your project, e.g., MIT, GPL, etc.)*
