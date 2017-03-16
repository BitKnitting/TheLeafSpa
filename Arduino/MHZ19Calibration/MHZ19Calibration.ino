// Grove - Co2 Sensor calibration

#include <SoftwareSerial.h>
SoftwareSerial sensor(7, 8);      // TX, RX
//MH-Z19 calibration command
const byte cmd_calibrate = {0xFF,0x01,0x87,0x00,0x00,0x00,0x00,0x00,0x78}; 
const unsigned char cmd_calibrate[] = 


void setup()
{
    sensor.begin(9600);
    Serial.begin(115200);
    Serial.println("begin to calibrate");

    for(int i=0; i<sizeof(cmd_calibrate); i++)
    {
        sensor.write(cmd_calibrate[i]);
    }

    Serial.println("calibrate done");
}

void loop()
{
    // nothing to do
}
