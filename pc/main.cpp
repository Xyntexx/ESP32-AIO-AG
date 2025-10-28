#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cstdlib>

#include "src/autosteer/autosteer.h"
#include "src/autosteer/was.h"
#include "src/autosteer/motor.h"
#include "src/autosteer/buttons.h"
#include "autosteer_simulator.h"

// Simple command parser
bool parseCommand(const std::string& cmd, AutosteerSimulator& sim) {
    if (cmd == "help" || cmd == "h") {
        std::cout << "Available commands:\n";
        std::cout << "  steer <angle>    - Set steering angle setpoint in degrees\n";
        std::cout << "  was <angle>      - Set WAS (actual angle) in degrees\n";
        std::cout << "  btn on/off       - Turn steer button on/off\n";
        std::cout << "  sw on/off        - Turn software switch on/off\n";
        std::cout << "  stats            - Show current statistics\n";
        std::cout << "  quit             - Exit the simulator\n";
        std::cout << "  help             - Show this help\n";
        return true;
    } 
    else if (cmd.substr(0, 5) == "steer ") {
        try {
            float angle = std::stof(cmd.substr(5));
            sim.setSteeringSetpoint(angle);
            std::cout << "Steering setpoint set to " << angle << " degrees\n";
        } catch (...) {
            std::cout << "Invalid angle value\n";
        }
        return true;
    }
    else if (cmd.substr(0, 4) == "was ") {
        try {
            float angle = std::stof(cmd.substr(4));
            sim.setWasAngle(angle);
            std::cout << "WAS angle set to " << angle << " degrees\n";
        } catch (...) {
            std::cout << "Invalid angle value\n";
        }
        return true;
    }
    else if (cmd == "btn on") {
        sim.setSteerButtonState(true);
        std::cout << "Steer button turned ON\n";
        return true;
    }
    else if (cmd == "btn off") {
        sim.setSteerButtonState(false);
        std::cout << "Steer button turned OFF\n";
        return true;
    }
    else if (cmd == "sw on") {
        sim.setSoftwareSwitchState(true);
        std::cout << "Software switch turned ON\n";
        return true;
    }
    else if (cmd == "sw off") {
        sim.setSoftwareSwitchState(false);
        std::cout << "Software switch turned OFF\n";
        return true;
    }
    else if (cmd == "stats") {
        sim.printStats();
        return true;
    }
    else if (cmd == "quit" || cmd == "exit" || cmd == "q") {
        return false;
    }
    else {
        std::cout << "Unknown command. Type 'help' for available commands.\n";
        return true;
    }
}

int main() {
    std::cout << "=== ESP32-AIO-AG Autosteer PC Testing Environment ===" << std::endl;
    
    // Initialize the simulator
    AutosteerSimulator simulator;
    if (!simulator.init()) {
        std::cerr << "Failed to initialize simulator" << std::endl;
        return 1;
    }
    
    std::cout << "Simulator initialized successfully. Type 'help' for available commands." << std::endl;

    // Start the autosteer loop in a separate thread
    bool running = true;
    std::thread autosteerThread([&]() {
        while (running) {
            simulator.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 20Hz update rate
        }
    });
    
    // Main command loop
    std::string cmd;
    while (running) {
        std::cout << "> ";
        std::getline(std::cin, cmd);
        
        running = parseCommand(cmd, simulator);
    }
    
    // Clean up
    autosteerThread.join();
    std::cout << "Simulator shutting down..." << std::endl;
    
    return 0;
} 