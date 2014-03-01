/*****************************************************************************
*
*  l_debug.h  - Low level simple UART output driver for driver debugging.
*
*****************************************************************************/
#ifndef __L_DEBUG_H__
#define __L_DEBUG_H__

#include "Arduino.h"
#include "pins_arduino.h"
#include <stdio.h>
#include <avr/pgmspace.h>


//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef	__cplusplus
extern "C" {
#endif

extern void ldbg_init(unsigned long baud);
extern void ldbg_putchar(char c);
extern void ldbg_putmem(char* mem, unsigned int size);
extern char ldbg_getchar(void);

#ifdef	__cplusplus
}
#endif

#endif //__L_DEBUG_H__


