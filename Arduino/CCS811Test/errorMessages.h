#include "errors.h"
#ifndef ERRORMESSAGES_H
#define ERRORMESSAGES_H
/****************************************************************************************************/
const char errorSuccess_msg[] PROGMEM = "Success";
const char errorI2cDataTooLong_msg[] PROGMEM = "I2C Error - send data too long.";
const char errorI2cAddrNAK_msg[] PROGMEM = {"I2C Error - couldn't find I2C address."};
const char errorI2cXmitNAK_msg[] PROGMEM = "I2C Error - received NACK when sending data.";
const char errorI2cOther_msg[] PROGMEM = "I2C Error - hmmm...some I2C error.";
const char errorI2cBegin_msg[] PROGMEM = "I2C Error - Check the wiring.";
const char errorI2cHelpersReadUInt_msg[] PROGMEM = "Could not read the unsigned int.";
const char errorSi7006Humidity_msg[] PROGMEM = "Si7006 humidity reading not valid.";
const char errorSi7006Temperature_msg[] PROGMEM = "Si7006 temperature reading not valid.";
const char errorUnknown_msg[] PROGMEM = "Unknown error.";

const char *const errorMsgTable[] PROGMEM = {errorSuccess_msg,errorI2cDataTooLong_msg,errorI2cAddrNAK_msg,
errorI2cXmitNAK_msg,errorI2cOther_msg,errorI2cBegin_msg,errorI2cHelpersReadUInt_msg,errorSi7006Humidity_msg,
errorSi7006Temperature_msg,errorUnknown_msg};
/****************************************************************************************************/

bool errorMessage(byte errCode,char *errorBuffer) {
  if (errCode < errorUnknown && errCode >= errorSuccess) {
    strcpy_P(errorBuffer, (char*)pgm_read_word(&(errorMsgTable[errCode])));
    return true;
  }
  return false;
}

#endif
