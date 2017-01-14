//
// DebugLib.cpp
// Margaret Johnson, January 2017
// Be Kind.
#include "DebugLib.h"

int freeRam () {
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval ==0 ? (int) &__heap_start : (int) __brkval);
}