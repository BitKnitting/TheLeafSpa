/* ----------------------------------------------------------------------
 * Copyright (C) 2014-2015 Cambridge CMOS Sensors. All rights reserved.
 *
 * $Date:        29 June 2016
 * $Revision: 	V.2016.06.29.1732
 *
 * Project: 	    CCS811 Example for Raspberry Pi
 * Title:	    api_ccs811_raspberry.c
 *
 * Description:	Demostration how to use CCS811 API on Raspberry Pi
 *
 * -------------------------------------------------------------------- */
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "api_ccs811.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int raspi_i2c_fd = 0;
int raspi_i2c_sht21_fd = 0;

static bool VERBOSE = false;

int output(const char *format, ...) {

    if (VERBOSE) printf(format);

    return 0;
}

void writeLogHeader(FILE *pFile, char* filename, char* app, char* boot, char* header) {
    fprintf(pFile, "========== HEADER ==========\r\n");
    fprintf(pFile, "LOGFILE = CCS_0.0.0\r\n");
    fprintf(pFile, "SENSOR = CCS811\r\n");
    fprintf(pFile, "DATECODE = N/A\r\n");
    fprintf(pFile, "DEVICE = PI_I2C\r\n");
    fprintf(pFile, "FILENAME = %s\r\n", filename);
    fprintf(pFile, "SOFTWARE = RaspberryPi_CCS811\r\n");
    fprintf(pFile, "VERSION = 2016.10.17.1047\r\n");
    fprintf(pFile, "AUTHOR = richard.lai@ams.com\r\n");
    fprintf(pFile, "UID = RaspberryPi\r\n");
    fprintf(pFile, "APPLICATION = %s\r\n", app);
    fprintf(pFile, "BOOTLOADER = %s\r\n", boot);
    fprintf(pFile, "HEADERS = %s\r\n", header);
    fprintf(pFile, "CONTINUED = 0\r\n");
    fprintf(pFile, "========== DATA ==========\r\n");
    fprintf(pFile, "%s\r\n", header);
}

uint8_t raspi_sleep(uint16_t ms) {
    output("raspi_sleep: %d\r\n", ms);
    return usleep(ms * 1000);
}

uint8_t _raspi_i2c_read(int i2c_fd, uint8_t reg, uint8_t size, byte_t *data) {
    int ret = 0;
    byte_t cmd[1] =  { reg };

    output("raspi_i2c_read: 0x%x, %d, 0x%x \r\n", reg, size, data);

    ret = write(i2c_fd, cmd, 1); usleep(50000);
    output("  + write reg: %d, %d\r\n", reg, ret);

    ret = read(i2c_fd, data, size); usleep(50000);
    output("  + read reg: %d, %d\r\n", reg, ret);

    for (int i = 0; i<ret; i++)
        output("    - [%d]: 0x%02x\r\n", i, *(data+i));

    if (i2c_fd == raspi_i2c_fd) {
    // FIXME: I DON'T KNOW WHAT THE HELL IS WRONG WITH THE FIRST DATA BYTE!!!
    if (reg == CONST_CCS811_REG_ALG_RESULT_DATA || reg == CONST_CCS811_REG_RAW_DATA)
        *(data) &= 0x7F;  
    }

    return ERROR_SUCCESS;
}
uint8_t raspi_i2c_read(uint8_t reg, uint8_t size, byte_t *data) {
    return _raspi_i2c_read(raspi_i2c_fd, reg, size, data);
}
uint8_t raspi_i2c_write(uint8_t size, byte_t *data) {
    output("raspi_i2c_write: 0x%x, %d, %02x \r\n", *data, size, data);
    output("  + write: %d, %d\r\n", *data, write(raspi_i2c_fd, data, size));
    usleep(50000);
    return ERROR_SUCCESS;
}

uint8_t gpio_wake_low() {
    digitalWrite(3, 0); usleep(50000);
    return ERROR_SUCCESS;
}
uint8_t gpio_wake_high() {
    digitalWrite(3, 1); usleep(50000);
    return ERROR_SUCCESS;
}
uint8_t gpio_reset_low() {
    digitalWrite(0, 0); usleep(50000);
    return ERROR_SUCCESS;
}
uint8_t gpio_reset_high() {
    digitalWrite(0, 1); usleep(50000);
    return ERROR_SUCCESS;
}

bool raspi_ccs811_init() {
    wiringPiSetup();

    pinMode(0, OUTPUT); //reset
    pullUpDnControl(2, PUD_UP);
    pinMode(2, INPUT); //int
    pinMode(3, OUTPUT); //wake

    printf("wiringPiI2CSetup: %d\r\n", raspi_i2c_fd = wiringPiI2CSetup(CCS811_I2C_ADDR));
    printf("wiringPiI2CSetup: %d\r\n", raspi_i2c_sht21_fd = wiringPiI2CSetup(0x40));

    if (raspi_i2c_fd >0) {

      i2c_read_t i2c_read = &raspi_i2c_read;
      i2c_write_t i2c_write = &raspi_i2c_write;
      sleep_t s = &raspi_sleep;

      ccs811_i2c_init(i2c_read, i2c_write, s);
      ccs811_gpio_wake_init(&gpio_wake_low, &gpio_wake_high);
      ccs811_gpio_reset_init(&gpio_reset_low, &gpio_reset_high);

      return true;

    }

    return false;
}

ccs811_environment_t sht21_read() {
    byte_t data[16];
    ccs811_environment_t env;

    _raspi_i2c_read(raspi_i2c_sht21_fd, 0xE3, 2, data);
    uint16_t t = (data[0] << 8) + data[1];
    env.temperature = (((double)175.72 / 65536) * t) - 46.85;

    _raspi_i2c_read(raspi_i2c_sht21_fd, 0xE5, 2, data);
    uint16_t h = (data[0] << 8) + data[1];
    env.humidity = (((double) 125 / 65536) * h) - 6;

    printf("SHT21: temperature = %.4f, humidity = %.4f\r\n", env.temperature, env.humidity);

   return env;
}

void ccs811_measurementLoop() {
    api_ccs811_reset();

    bool isAppValid = false;
    printf("api_ccs811_is_app_valid: 0x%x\r\n", isAppValid = api_ccs811_is_app_valid());

    if (!isAppValid) {
        printf("CCS811 not avaialable\r\n");
	return;
    }

    uint16_t bootloader = 0;
    uint16_t application = 0;

    api_ccs811_read_version_bootloader(&bootloader);
    api_ccs811_read_version_application(&application);
    printf("api_ccs811_read_version_bootloader: 0x%04x\r\n", bootloader);
    printf("api_ccs811_read_version_application: 0x%04x\r\n", application);

    if (!isAppValid) {
        printf("CCS811 Application not valid.\r\n");
        return;
    } else {
        api_ccs811_start_application();
    }

    bool isAppRunning = false;
    printf("api_ccs811_is_app_running: 0x%x\r\n", isAppRunning = api_ccs811_is_app_running());
    if (!isAppRunning) return;

    api_ccs811_set_measurement_mode(CONST_MEASUREMENT_MODE_VOC, true);

    ccs811_measurement_t ccs811;
    char logFilename[255];
    sprintf(logFilename, "CCS811_%d.csv", (unsigned)time(NULL));

    bool ready = false;
    int c;
    for(;;) {
        ready = digitalRead(2)==0; printf(".\r");
        if (ready) {

            if (ERROR_SUCCESS != api_ccs811_read_measurement(&ccs811)) continue;
            ccs811_environment_t env = sht21_read();
            api_ccs811_set_environment_data(env);

            char* data;
            printf(data = "measurement: ADC = %d, CURRENT = %d, RESISTANCE = %.0f, VOC = %d PPM, TVOC = %d PPB\r\n", ccs811.adc, ccs811.current, ccs811.resistance, ccs811.voc, ccs811.tvoc);
            FILE *pFile;
            bool newFile = false;
            if (!(pFile = fopen(logFilename, "r"))) {
                newFile = true;
            } else fclose(pFile);
            if (pFile = fopen(logFilename, "a")) {
                if (newFile) {
                    char app[255];  sprintf(app, "%d.%d.%d", (application >> 4) & 0xf, application & 0xf, (application >> 8) & 0xff);
                    char boot[255]; sprintf(boot, "%d.%d.%d", (bootloader & 0xf) >> 4, bootloader & 0xf, (bootloader >> 8) & 0xff);
                    char header[255]; sprintf(header, "%-25s, %-20s, %-20s, %-20s, %-20s, %-20s, %-20s, %-20s", "date_time", "temperature [°C]", "humidity [%]", "resistance [ohm]", "eCO2 [PPM]", "TVOC [PPB]", "adc", "current");
                    writeLogHeader(pFile, logFilename, app, boot, header);
                }

                struct tm* tm_info;
                struct timeval tv;
                gettimeofday(&tv, NULL);
                int millisec = lrint(tv.tv_usec/1000.0);
                if (millisec >= 1000) { millisec -= 1000; tv.tv_sec++; }

                tm_info = localtime(&tv.tv_sec);

                //time_t timer;
                //time(&timer);
                char dt[26];
                strftime(dt, 26, "%Y-%m-%d_%H:%M:%S", tm_info/*(localtime(&timer)*/);
                sprintf(dt, "%s.%03d", dt, millisec);
                fprintf(pFile, "%s, %-20.5f, %-20.5f, %-20.0f, %-20.0f, %-20.0f, %-20d, %-20d\r\n", dt, env.temperature, env.humidity, ccs811.resistance, ccs811.voc, ccs811.tvoc, ccs811.adc, ccs811.current);
                fclose(pFile);
                printf("%sfile written: %s\r\n", newFile ? "New " : "", logFilename);
            } else printf("no File written: 0x%x\r\n", pFile);
        }
        usleep(200000);
    }
}


void show_menu(bool has811, char c) {
    int input;
    char menu_text[255];

    input = c;

    while(1) {
        system("@cls||clear");

        bool isAppValid = false;
        uint16_t bootloader = 0;
        uint16_t application = 0;
        char version[64];
        if (has811) {
            api_ccs811_read_version_bootloader(&bootloader);

            if (isAppValid = api_ccs811_is_app_valid()) api_ccs811_read_version_application(&application);
            sprintf(version, "CCS811: Bootloader = %d.%d.%d, Application = %d.%d.%d",
                     (bootloader & 0xf) >> 4, bootloader & 0xf, (bootloader >> 8) & 0xff,
                     (application >> 4) & 0xf, application & 0xf, (application >> 8) & 0xff);
        }

        sprintf(menu_text, "%s%s\r\n\r\n%s%s%s%s",
                        "Cambridge CMOS Sensors. Copyright 2016. All rights reserved.\r\n\r\n",
                        !has811 ? "CCS811 device not be found or validated." : version,
                        !has811 ? "" : "  [D] Update device application image (CCS811.BIN)\r\n",
                        !has811 ? "  [R] Scan for CCS811 device \r\n" : isAppValid ? "  [M] Start Measurement \r\n" : "",
                        VERBOSE ? "  [V] Disable verbose message\r\n" : "  [V] Enable verbose message\r\n",
                        "  [X] Exit \r\n\r\n");

        printf("%sPlease select option[%d,%d]: ", menu_text, c, input);
        if (input == 0) while ((input = getc(stdin)) < '0') ;

        switch (input) {
          case 'd':
          case 'D':
              printf("\r\nStarting firmware update. Please wait...\r\n");
              if( access( "ccs811.bin", F_OK ) != -1 ) {
                  FILE *fileptr;
                  uint8_t *buffer;
                  long filelen;
                  long fileread = 0;

                  fileptr = fopen("ccs811.bin", "rb");  // Open the file in binary mode
                  if (fileptr == NULL) {
                      printf("\r\n### Failed to open CCS811.BIN. Press ENTER to return to the main menu."); getchar();
                      break;
                  }

                  fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
                  filelen = ftell(fileptr);             // Get the current byte offset in the file
                  rewind(fileptr);                      // Jump back to the beginning of the file

                  if (filelen < 1) {
                      printf("\r\n### Invalid CCS811.BIN size (%d). Press ENTER to return to the main menu.", filelen); getchar();
                      break;
                  }

                  buffer = (char *)malloc((filelen+1)*sizeof(char)); // Enough memory for file + \0
                  if (buffer != NULL) {
                      if ((fileread = fread(buffer, 1, filelen, fileptr)) != filelen) {
                          printf("\r\n### Read CCS811.BIN error (size mismatch [%d, %d]). Press ENTER to return to the main menu.", filelen, fileread);
                          getchar();
                      } else has811 = api_ccs811_write_image(filelen, buffer) == ERROR_SUCCESS;

                      fclose(fileptr); // Close the file
                      free(buffer);

                      printf("\r\nFirmware update completed[%d]. Press any key to return to main menu.\r\n", has811);
                  }

              } else {
                  printf("\r\n### Image file CCS811.BIN doesn't exist. Press ENTER to return to the main menu.");
              }
              getchar();
              break;
          case 'm':
          case 'M':
              if (has811) ccs811_measurementLoop();
              break;
          case 'r':
          case 'R':
              // FIXME: Will need to reinitialise raspi_i2c_fd as well
              api_ccs811_reset();
              show_menu(api_ccs811_is_app_valid(), (char)0);
              return;
          case 'v':
          case 'V':
              VERBOSE = !VERBOSE;
              break;
          case 'x':
          case 'X':
              exit(0);
        }
        while (getchar() != '\n');
        input = 0;
    }
}

int main(int argc, char *argv[]) {

    bool has811 = false;

    if (argc > 1) printf("main: %d, %c\r\n", argc, *(argv[1]));

    if (raspi_ccs811_init()) {
        api_ccs811_reset();
        printf("api_ccs811_is_app_valid: 0x%2X\r\n", has811 = api_ccs811_is_valid());
    }

   show_menu(has811, argc > 1 ? *(argv[1]) : (char)0);

    return 0;
}

