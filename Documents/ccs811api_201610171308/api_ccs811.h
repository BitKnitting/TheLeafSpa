/* ----------------------------------------------------------------------
 * Copyright (C) 2014-2015 Cambridge CMOS Sensors. All rights reserved.
 *
 * $Date:        29 June 2016
 * $Revision: 	V.2016.06.29.1732
 *
 * Project: 	    CCS811 API
 * Title:	    api_ccs811.h
 *
 * Description:	Header API for CCS811
 *
 * -------------------------------------------------------------------- */
#ifndef _API_CCS811_H_
#define _API_CCS811_H_

typedef unsigned char byte_t;
typedef struct {
    uint16_t voc;
    uint16_t tvoc;
    uint16_t adc;
    uint16_t current;
    double resistance;
} ccs811_measurement_t;
typedef struct {
    double humidity;
    double temperature;
} ccs811_environment_t;
typedef struct {
    uint16_t mid;
    uint16_t high;
} ccs811_thresholds_t;
#include <stdbool.h>

/**
* @brief Function prototype
*/
// Parameters: I2C Address, Size, Data Buffer
typedef uint8_t(*i2c_read_t)(uint8_t,uint8_t,byte_t*);
// Parameters: I2C Address, Size, Data Buffer
typedef uint8_t(*i2c_write_t)(uint8_t,byte_t*);
// Parameters: sleep time in ms
typedef uint8_t(*sleep_t)(uint16_t);
typedef uint8_t(*gpio_wake_pull_low_t)(void);
typedef uint8_t(*gpio_wake_pull_high_t)(void);
typedef uint8_t(*gpio_reset_pull_low_t)(void);
typedef uint8_t(*gpio_reset_pull_high_t)(void);

/**
* @brief CCS811 API functions
*/
void ccs811_gpio_wake_init(gpio_wake_pull_low_t l, gpio_wake_pull_high_t h);
void ccs811_gpio_reset_init(gpio_reset_pull_low_t l, gpio_reset_pull_high_t h);
void ccs811_i2c_init(i2c_read_t r, i2c_write_t w, sleep_t s);
uint8_t ccs811_read(uint8_t reg, uint8_t len, byte_t *p);
uint8_t ccs811_read_status(uint8_t *status);
uint8_t ccs811_write(uint8_t len, byte_t *p);
uint8_t ccs811_write_reg(uint8_t reg);
void api_ccs811_reset();
bool api_ccs811_is_valid();
bool api_ccs811_is_app_valid();
bool api_ccs811_is_app_running();
uint8_t api_ccs811_start_application();
uint8_t api_ccs811_read_version_hardware(uint16_t *version);
uint8_t api_ccs811_read_version_bootloader(uint16_t *version);
uint8_t api_ccs811_read_version_application(uint16_t *version);
uint8_t api_ccs811_set_measurement_mode(uint8_t mode, bool interrupt);
uint8_t api_ccs811_read_measurement(ccs811_measurement_t *measurement);
uint8_t api_ccs811_set_environment_data(ccs811_environment_t environment);
uint8_t api_ccs811_write_image(uint16_t length, uint8_t *image);

#define CCS811_I2C_ADDR 0x5A

/**
* @brief CCS811 API Constants
*/
#define CONST_CCS811_REG_STATUS 0x00
#define CONST_CCS811_REG_MEAS_MODE 0x01
#define CONST_CCS811_REG_ALG_RESULT_DATA 0x02
#define CONST_CCS811_REG_RAW_DATA 0x03
#define CONST_CCS811_REG_ALG_CONFIG_DATA 0x04
#define CONST_CCS811_REG_ENVIRONMENT_DATA 0x05;
#define CONST_CCS811_REG_THRESHOLDS 0x10;

#define CONST_CCS811_REG_HW_ID 0x20
#define CONST_CCS811_REG_HW_VER 0x21
#define CONST_CCS811_REG_HW_UID 0x22
#define CONST_CCS811_REG_FW_BOOT_VER 0x23
#define CONST_CCS811_REG_FW_APP_VER 0x24

#define CONST_CCS811_REG_BOOT_APP_ERASE 0xF1
#define CONST_CCS811_REG_BOOT_APP_DATA 0xF2
#define CONST_CCS811_REG_BOOT_APP_VERIFY 0xF3
#define CONST_CCS811_REG_BOOT_APP_START 0xF4

#define ERROR_SUCCESS 0
#define ERROR_FAILED 1
#define ERROR_TIMEOUT 2

/* Status register bit-fields */
/* Bit 7 - Firmware mode */
#define CONST_CCS811_STATUS_FW_MODE_MASK        0x80
#define CONST_CCS811_STATUS_FW_MODE_BOOT        0x00
#define CONST_CCS811_STATUS_FW_MODE_APP         0x80
/* Bit 6 - Erased, waiting for programming */
#define CONST_CCS811_STATUS_ERASED_MASK         0x40
#define CONST_CCS811_STATUS_ERASED_TRUE         0x40
#define CONST_CCS811_STATUS_ERASED_FALSE        0x00
/* Bit 5 - CRC Check has been performed, whether successful or not */
#define CONST_CCS811_STATUS_CRC_CHECKED_MASK    0x20
#define CONST_CCS811_STATUS_CRC_CHECKED_TRUE    0x20
#define CONST_CCS811_STATUS_CRC_CHECKED_FALSE   0x00
/* Bit 4 - Valid application is present */
#define CONST_CCS811_STATUS_APP_VALID_MASK      0x10
#define CONST_CCS811_STATUS_APP_VALID_TRUE      0x10
#define CONST_CCS811_STATUS_APP_VALID_FALSE     0x00
/* Bit 3 - New result/data available */
#define CONST_CCS811_STATUS_DATA_READY_MASK     0x08
#define CONST_CCS811_STATUS_DATA_READY_TRUE     0x08
#define CONST_CCS811_STATUS_DATA_READY_FALSE    0x00
/* Bit 2 - Threshold exceeded? */
#define CONST_CCS811_STATUS_THRESH_MASK         0x04
#define CONST_CCS811_STATUS_THRESH_TRUE         0x04
#define CONST_CCS811_STATUS_THRESH_FALSE        0x00
/* Bit 1 - reserved */
/* Bit 0 - Error Indication? */
#define CONST_CCS811_STATUS_ERR_MASK            0x01
#define CONST_CCS811_STATUS_ERR_TRUE            0x01
#define CONST_CCS811_STATUS_ERR_FALSE           0x00

#define CONST_CCS811_STATUS_APP_RUNNING \
        (CONST_CCS811_STATUS_APP_VALID_TRUE | CONST_CCS811_STATUS_FW_MODE_APP)

#define CONST_MEASUREMENT_MODE_IDLE             0x00
#define CONST_MEASUREMENT_MODE_VOC              0x10
#define CONST_MEASUREMENT_MODE_VOC_10S          0x20
#define CONST_MEASUREMENT_MODE_VOC_60S          0x30
#define CONST_MEASUREMENT_MODE_VOC_16V          0x90

#endif // _API_CCS811_H_
