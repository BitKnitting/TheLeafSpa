---------------- GrowOS v0.991 beta ----------------
----------- Casabonita420 - 28/06/2016 -----------

----------
- Switch the Co2 sensor to UART. There's a I2C file provided but it's obsolete.
- tweaked twi.c, replace the file in:
C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\Wire\utility
----------

The basic operation of GrowOS is to enrich Co2 while keeping temperature/humidity 
to acceptable levels. If Temp/Hum exceed these levels, CO2 enrichment stops and 
the exhaust fan is turned on until temperature and humidity are brought down.

If it's dark, GrowOS keeps the exhaust vent running on the LOW setting while monitoring 
for Temperature and Humidity levels. If Max Temp is met the exhaust fan 
kicks in at HIGH setting. If variable Dehum is met the dehumidifier is turned on. 
If the dehumidifier is unable to control humidity and Max Hum is met the exhaust 
fan kicks in at HIGH setting.

If it's day, GrowOS checks CO2 PPM levels, pumps CO2 and maintains it at a predefined
level (e.g. 1500ppm). If levels drop below the predefined delta ppm value (e.g 200ppm) 
the CO2 is engaged to bring levels up to 1500 again. While this constant CO2 
monitoring/spraying routine is engaged the system keeps track of Temperature and 
Humidity levels as well. If the dehumidifier is unable to bring humidity levels to 
an acceptable range the CO2 stops and the exhaust fan kicks in at HIGH setting as 
a last resort until we have acceptable temperature or humidity levels for CO2 enrichment.

The display shows Temperature, Humidity, CO2 PPM and the state of light and each relay.
"L" for Light (0/1), "F" for Fan (0/Low/High), "D" for Dehumidifier (0/1) and "C" for CO2 (0/1).
A rotary encoder is used to program the device. If the button is pressed for longer
than 1.5 seconds GrowOS enters programming mode.
In programming mode you are requested to enter the values for:
Max Temp, Delta Temp, Max Hum, Delta Hum, Dehum, Delta Dehum, Max PPM, Delta PPM.
The device goes through each value and saves on exit but you can skip right to exit&save 
with a long press of the button.

To describe"Delta" I will give an example: If we have our Max Temp at 35c and our 
Delta Temp at 10c, the exhaust vent will engage when temperature exceeds 35c but 
will only stop when temperature is brought down to 25c (35 - 10 = 25).

The relay configuration:
- Relay 1 controls Exhaust fan LOW/HIGH setting
- Relay 2 controls Exhaust fan power ON/OFF
- Relay 3 controls Dehumidifier power ON/OFF 
- Relay 4 controls CO2 solenoid valve ON/OFF 

Use a 7-7.5vdc power supply for the arduino and avoid a 12v so the regulator won't get hot.
Also use a 5v power supply inside the relay box. 
