///
/// DebugLib.h
/// Margaret Johnson January, 2017
/// Be Kind.
#ifndef DebugLib_h
#define DebugLib_h
#ifdef DEBUG
#define DEBUG_BEGIN Serial.begin(115200)
#define DEBUG_PRINT(x)  Serial.print (x)
#define DEBUG_PRINTHEX(x) Serial.print(x,HEX)
#define DEBUG_PRINTF(x) Serial.print(F(x))
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTLNF(x) Serial.println(F(x))
#else
#define DEBUG_BEGIN
#define DEBUG_PRINT(x)
#define DEBUG_PRINTF(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTLNF(x)
#define DEBUG_PRINTHEX(x)
#endif
int freeRam ();



#endif
