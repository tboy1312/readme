#include "modbus_operations.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <stdint.h>
#include <modbus/modbus.h>

// Global Modbus context
modbus_t *ctx = nullptr;

void initializeModbusContext() {
    if (ctx == nullptr) {
        ctx = modbus_new_tcp("169.254.234.100", 502);
        //modbus_set_debug(ctx, TRUE);
        if (modbus_connect(ctx) == -1) {
            std::cerr << "Connection to Modbus failed: " << modbus_strerror(errno) << std::endl;
            modbus_free(ctx);
            ctx = nullptr;
        }
    }
}

int calculate_address(int address) {
    if (address >= 400001) {
        return address - 400001;
    }
    // Extend with other address ranges if needed
    return address;
}

int calculate_coil_address(int address) {
    if (address >= 100001) {
        return address - 100001;
    }
    return address;
}

int read_from_modbus(int address) {
    int adjusted_address = calculate_address(address);
    uint16_t data[1];
    int res = modbus_read_registers(ctx, adjusted_address, 1, data);
    if (res == -1) {
        std::cerr << "Failed to read: " << modbus_strerror(errno) << std::endl;
        return -1; // Indicate error
    }
    return data[0];
}

bool write_to_modbus(int address, int value) {
    int adjusted_address = calculate_address(address);
    int res = modbus_write_register(ctx, adjusted_address, value);
    if (res == -1) {
        std::cerr << "Failed to write: " << modbus_strerror(errno) << std::endl;    
        return false;
    }
    return true;
}

bool read_coil_from_modbus(int address) {
    int adjusted_address = calculate_coil_address(address);
    uint8_t data;
    int res = modbus_read_bits(ctx, adjusted_address, 1, &data);
    if (res == -1) {
        std::cerr << "Failed to read coil: " << modbus_strerror(errno) << std::endl;
        return -1; // Indicate error
    }
    return (bool)data;
}

bool write_coil_to_modbus(int address, bool value) {
    int adjusted_address = calculate_coil_address(address);
    int res = modbus_write_bit(ctx, adjusted_address, value);
    if (res == -1) {
        std::cerr << "Failed to write coil: " << modbus_strerror(errno) << std::endl;
        return false;
    }
    return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void continuously_read_register(int address) {
    while (true) {  // Infinite loop
        int register_value = read_from_modbus(address);
        
        // If you want to handle errors (e.g., when read_from_modbus returns -1), you can add an if condition here.
        if (register_value == -1) {
            std::cerr << "Error reading address " << address << ". Retrying in 1 second..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));  // 1 second delay before retrying
            continue;  // Skip to the next iteration
        }

        // Optional: Add a delay between readings
        std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 500ms delay
    }
}

void continuously_read_coil(int address) {
    while (true) {  // Infinite loop
        bool coil_value = read_coil_from_modbus(address);
        
        // Optional: Add a delay between readings
        std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 500ms delay
    }
}

int read_multiple_registers(int start_address, int num_registers, unsigned short* buffer) {
    int adjusted_address = calculate_address(start_address);
    int res = modbus_read_registers(ctx, adjusted_address, num_registers, buffer);
    if (res == -1) {
        std::cerr << "Failed to read registers: " << modbus_strerror(errno) << std::endl;
        return -1; // Indicate error
    }
    return res; // Returns the number of registers read
}


void continuously_read_multiple_registers(int start_address, int num_registers) {
    uint16_t buffer[num_registers];
    
    while (true) {  // Infinite loop
        int read_count = read_multiple_registers(start_address, num_registers, buffer);
        
        if (read_count == -1) {
            std::cerr << "Error reading starting from address " << start_address << ". Retrying in 1 second..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));  // 1 second delay before retrying
            continue;  // Skip to the next iteration
        }
        
        for (int i = 0; i < read_count; i++) {
            std::cout << start_address + i << ": " << buffer[i] << std::endl;
        }
        
        // Optional: Add a delay between readings
        std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 500ms delay
    }
}
