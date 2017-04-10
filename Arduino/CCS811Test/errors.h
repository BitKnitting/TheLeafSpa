#ifndef ERRORS_H
#define ERRORS_H
/****************************************************************************************************/
/*
 * NOTE: The order of the error messages matter:
 * -> success should be 0.
 * -> the wire library returns errors 1 - 4
 * -> the order is matched to the error message table within errorMessages.h
 */
enum errors_t {
  errorSuccess,
  errorI2cDataTooLong,
  errorI2cAddrNAK,
  errorI2cXmitNAK,
  errorI2cOther,
  errorI2cBegin,
  errorI2cHelpersReadUInt,
  errorSi7006Humidity,
  errorSi7006Temperature,
  errorUnknown
};


#endif
