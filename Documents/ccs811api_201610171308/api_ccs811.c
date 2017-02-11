/* ----------------------------------------------------------------------
 * Copyright (C) 2014-2015 Cambridge CMOS Sensors. All rights reserved.
 *
 * $Date:        29 June 2016
 * $Revision: 	V.2016.06.29.1732
 *
 * Project: 	    CCS811 API
 * Title:	    api_ccs811.c
 *
 * Description:	C API for CCS811
 *
 * -------------------------------------------------------------------- */
#include <stdio.h>
#include <stdint.h>
#include "api_ccs811.h"

static i2c_read_t ic2_read;
static i2c_write_t ic2_write;
static gpio_wake_pull_low_t wake_low;
static gpio_wake_pull_high_t wake_high;
static gpio_reset_pull_low_t reset_low;
static gpio_reset_pull_high_t reset_high;
static sleep_t sleep;

/**
* @brief Caller should map platform specific CCS811 WAKE pin operation via this API call
*/
void ccs811_gpio_wake_init(gpio_wake_pull_low_t l, gpio_wake_pull_high_t h) {
    wake_low = l;
    wake_high = h;
}

/**
* @brief Caller should map platform specific CCS811 RESET pin operation via this API call
*/
void ccs811_gpio_reset_init(gpio_reset_pull_low_t l, gpio_reset_pull_high_t h) {
    reset_low = l;
    reset_high = h;
}

/**
* @brief Caller should map platform specific CCS811 IC2 read/write operation via this API call
*/
void ccs811_i2c_init(i2c_read_t r, i2c_write_t w, sleep_t s) {
    ic2_read = r;
    ic2_write = w;
    sleep = s;
}

/**
* @brief API call to read data from CCS811
*/
uint8_t ccs811_read(uint8_t reg, uint8_t len, byte_t *p) {
    return ic2_read(reg, len, p);
}

/**
* @brief API call to write data to CCS811
*/
uint8_t ccs811_write(uint8_t len, byte_t *p) {
    return ic2_write(len, p);
}

/**
* @brief API call to execute command on CCS811
*/
uint8_t ccs811_write_reg(uint8_t reg) {
    uint8_t w[1] = { reg };
    return ic2_write(1, (uint8_t*)&w);
}

/**
* @brief API call to read CCS811 status register
*/
uint8_t ccs811_read_status(uint8_t *status) {
    return ccs811_read(CONST_CCS811_REG_STATUS, 1, status);
}


/**
* @brief API call check application on CCS811 if it is valid or not
*/
void api_ccs811_reset() {
    wake_low();   sleep(10);
    reset_low();  sleep(10);
    reset_high(); sleep(10);
    wake_high();  sleep(10);
}

/**
* @brief API call check if CCS811 device is present
*/
bool api_ccs811_is_valid() {
    uint8_t status = 0;
    uint8_t ret = ERROR_FAILED;

    wake_low();

    ret = ccs811_read(CONST_CCS811_REG_HW_ID, 1, &status);;
    if (ret != ERROR_SUCCESS) goto Finish;
    ret = 0x81 == (status & 0x81);

    Finish:
        wake_high();

    return ret;
}

/**
* @brief API call check application on CCS811 if it is valid or not
*/
bool api_ccs811_is_app_valid() {
    uint8_t status = 0;
    uint8_t ret = ERROR_FAILED;

    wake_low();

    ret = ccs811_read_status(&status);
    if (ret != ERROR_SUCCESS) goto Finish;

    ret = CONST_CCS811_STATUS_APP_VALID_TRUE == (status & CONST_CCS811_STATUS_APP_VALID_TRUE);

    Finish:
        wake_high();

    return ret;
}

/**
* @brief API call to check if application is running on CCS811 or not
*/
bool api_ccs811_is_app_running() {
    uint8_t status = 0;
    uint8_t ret = ERROR_FAILED;

    wake_low();

    ret = ccs811_read_status(&status);
    if (ret != ERROR_SUCCESS) goto Finish;

    ret = CONST_CCS811_STATUS_APP_RUNNING == (status & CONST_CCS811_STATUS_APP_RUNNING);

    Finish:
        wake_high();

    return ret;
}

/**
* @brief API call to start application on CCS811
*/
uint8_t api_ccs811_start_application() {
    uint8_t status = 0;
    uint8_t ret = ERROR_FAILED;

    wake_low();

    ret = ccs811_read_status(&status);
    if (ret != ERROR_SUCCESS) goto Finish;
    else if (CONST_CCS811_STATUS_APP_RUNNING == (status & CONST_CCS811_STATUS_APP_RUNNING)) {
        // App already running
        ret = ERROR_FAILED;
        goto Finish;
    } else if (CONST_CCS811_STATUS_APP_VALID_TRUE != (status & CONST_CCS811_STATUS_APP_VALID_TRUE)) {
        // App not valid
        ret = ERROR_FAILED;
        goto Finish;
    }

    ret = ccs811_write_reg(CONST_CCS811_REG_BOOT_APP_START); sleep(200);
    if (ret != ERROR_SUCCESS) goto Finish;


    ret = ccs811_read_status(&status);
    if (ret != ERROR_SUCCESS) goto Finish;
    if (CONST_CCS811_STATUS_APP_RUNNING != (status & CONST_CCS811_STATUS_APP_RUNNING)) {
        // App not running
        ret = ERROR_FAILED;
        goto Finish;
    }

    Finish:
        wake_high();

    return ret;
}

/**
* @brief API call to read CCS811 device version
*/
uint8_t api_ccs811_read_version_hardware(uint16_t *version) {
    uint8_t ret = ERROR_SUCCESS;

    wake_low();
    ret = ccs811_read(CONST_CCS811_REG_HW_ID, 2, (uint8_t*)version);
    wake_high();

    return ret;
}

/**
* @brief API call to read CCS811 bootloader version
*/
uint8_t api_ccs811_read_version_bootloader(uint16_t *version) {
    uint8_t ret = ERROR_SUCCESS;

    wake_low();
    ret = ccs811_read(CONST_CCS811_REG_FW_BOOT_VER, 2, (uint8_t*)version);
    wake_high();

    return ret;
}

/**
* @brief API call to read CCS811 application version
*/
uint8_t api_ccs811_read_version_application(uint16_t *version) {
    uint8_t ret = ERROR_SUCCESS;

    wake_low();
    ret = ccs811_read(CONST_CCS811_REG_FW_APP_VER, 2, (uint8_t*)version);
    wake_high();

    return ret;
}

/**
* @brief API call to set measurement mode (CONST_MEASUREMENT_MODE_XXX) on CCS811
*/
uint8_t api_ccs811_set_measurement_mode(uint8_t mode, bool interrupt) {
    uint8_t status = 0;
    uint8_t ret = ERROR_SUCCESS;
    uint8_t w[2] = { CONST_CCS811_REG_MEAS_MODE, mode + (interrupt ? CONST_CCS811_STATUS_DATA_READY_MASK : 0) };

    wake_low();

    ret = ccs811_read_status(&status);
    if (ret != ERROR_SUCCESS) goto Finish;

    ret = ccs811_write(2, (uint8_t*)&w);
    if (ret != ERROR_SUCCESS) goto Finish;

    ret = CONST_CCS811_STATUS_FW_MODE_APP == (status & CONST_CCS811_STATUS_FW_MODE_APP);
    if (ret != ERROR_SUCCESS) goto Finish;

    sleep(1000);

    Finish:
        wake_high();

    return ret;
}

/**
* @brief API call to read measurement data from CCS811
*/
uint8_t api_ccs811_read_measurement(ccs811_measurement_t *measurement) {
    uint8_t status = ERROR_SUCCESS;
    uint8_t ret = 0;

    uint8_t result[4] = {0};
    uint8_t *p = (uint8_t*)&result;
    measurement->voc = 0;
    measurement->tvoc = 0;
    measurement->adc = 0;
    measurement->current = 0;
    measurement->resistance = 0;

    wake_low();

    ret = ccs811_read_status(&status);
    if (ret != ERROR_SUCCESS ) goto Finish;
    else if (CONST_CCS811_STATUS_DATA_READY_TRUE != (status & CONST_CCS811_STATUS_DATA_READY_TRUE)) return false;

    ret = ccs811_read(CONST_CCS811_REG_ALG_RESULT_DATA, 4, p);
    if (ret != ERROR_SUCCESS ) goto Finish;
    measurement->voc = *(p + 1) + (*(p+0) << 8);
    measurement->tvoc = *(p + 3) + (*(p + 2) << 8);

    ret = ccs811_read(CONST_CCS811_REG_RAW_DATA, 2, p);
    if (ret != ERROR_SUCCESS ) goto Finish;
    measurement->adc = *(p + 1) + ((*(p) & 0x3) << 8);
    measurement->current = (*(p) >> 2) & 0x3f;

    measurement->resistance = ((1.65 * ((double)measurement->adc / 0x3ff)) / measurement->current) * 1000000;

    Finish:
        wake_high();

    return ERROR_SUCCESS;
}

/**
* @brief Write back environment data to CCS811
*/
uint8_t api_ccs811_set_environment_data(ccs811_environment_t environment) {
    uint8_t ret = ERROR_SUCCESS;
    byte_t payload[5];

    wake_low();

    payload[0] = CONST_CCS811_REG_ENVIRONMENT_DATA;
    payload[1] = (byte_t)(environment.humidity * 2);
    payload[2] = 0;
    payload[3] = (byte_t)((environment.temperature + 25) * 2);
    payload[4] = 0;

    ret = ccs811_write(5, (uint8_t*)&payload);
    if (ret != ERROR_SUCCESS) goto Finish;

    Finish:
        wake_high();

    return ret;
}

/**
* @brief Set interrupt thresholds. When set, interrupted is triggered only when crossing threshold levels
*/
uint8_t api_ccs811_set_thresholds(ccs811_thresholds_t thresholds) {
    uint8_t ret = ERROR_SUCCESS;
    byte_t payload[5];

    wake_low();

    payload[0] = CONST_CCS811_REG_THRESHOLDS;
    payload[1] = (byte_t)(thresholds.mid >> 8);
    payload[2] = (byte_t)(thresholds.mid & 0xff);
    payload[3] = (byte_t)(thresholds.high >> 8);
    payload[4] = (byte_t)(thresholds.high & 0xff);

    ret = ccs811_write(5, (uint8_t*)&payload);
    if (ret != ERROR_SUCCESS) goto Finish;

    Finish:
        wake_high();

    return ret;
}

/**
* @brief API call to download new bootloader/application to CCS811
*/
uint8_t api_ccs811_write_image(uint16_t length, uint8_t *image) {
    uint8_t status = 0;
    uint8_t ret = 0;
    uint16_t version = 0;
    byte_t payload[9];
    payload[0] = CONST_CCS811_REG_BOOT_APP_DATA;
    byte_t CCS811_ERASE[] = { CONST_CCS811_REG_BOOT_APP_ERASE, 0xE7, 0xA7, 0xE6, 0x09 };

    wake_low(); sleep(50);

    // Pulse Reset Pin
    reset_low(); sleep(100);
    reset_high(); sleep(100);

    ret = ccs811_write(5, (byte_t*)&CCS811_ERASE);
    if (ret != ERROR_SUCCESS) goto Finish;
    sleep(500);

    // Read file in 8-byte chunks until the end is reached, and send each to the board
    for (int i = 0; i < length; i+=8)
    {
        for (int j = 0; j < 8; j++)
        {
            payload[j + 1] = *(image + i + j);
        }
        ret = ccs811_write(9, (byte_t*)&payload);
        if (ret != ERROR_SUCCESS) goto Finish;
        sleep(50);
    }

    ret = ccs811_write_reg(CONST_CCS811_REG_BOOT_APP_VERIFY); sleep(500);
    if (ret != ERROR_SUCCESS) goto Finish;

    ret = ccs811_read_status(&status);
    if (ret != ERROR_SUCCESS) goto Finish;

    // Pulse Reset Pin
    reset_low(); sleep(100);
    reset_high(); sleep(100);

    ret = CONST_CCS811_STATUS_APP_VALID_TRUE == (status & CONST_CCS811_STATUS_APP_VALID_TRUE) ? ERROR_SUCCESS : ERROR_FAILED;
    if (ret != ERROR_SUCCESS) goto Finish;

    Finish:
        wake_high();

    return ret;
}
