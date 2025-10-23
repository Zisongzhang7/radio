#include "i2c_device.h"

#include <esp_log.h>

#define TAG "I2cDevice"


I2cDevice::I2cDevice(i2c_master_bus_handle_t i2c_bus, uint8_t addr) {
    i2c_device_config_t i2c_device_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = 400 * 1000,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = 0,
        },
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &i2c_device_cfg, &i2c_device_));
    assert(i2c_device_ != NULL);
}

bool I2cDevice::WriteReg(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    esp_err_t ret = i2c_master_transmit(i2c_device_, buffer, 2, 100);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "I2C写寄存器失败 (reg=0x%02X): %s", reg, esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool I2cDevice::ReadReg(uint8_t reg, uint8_t& value) {
    uint8_t buffer[1];
    esp_err_t ret = i2c_master_transmit_receive(i2c_device_, &reg, 1, buffer, 1, 100);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "I2C读寄存器失败 (reg=0x%02X): %s", reg, esp_err_to_name(ret));
        return false;
    }
    value = buffer[0];
    return true;
}

bool I2cDevice::ReadRegs(uint8_t reg, uint8_t* buffer, size_t length) {
    esp_err_t ret = i2c_master_transmit_receive(i2c_device_, &reg, 1, buffer, length, 100);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "I2C读多个寄存器失败 (reg=0x%02X, len=%d): %s", reg, length, esp_err_to_name(ret));
        return false;
    }
    return true;
}