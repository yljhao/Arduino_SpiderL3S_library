/*****************************************************************************
*
*  spider_spi.h  - CC3000 Host Driver Implementation.
*
*****************************************************************************/
#ifndef __SPIDER_SPI_H__
#define __SPIDER_SPI_H__

#include "Arduino.h"
#include "avr/pgmspace.h"
#include "pins_arduino.h"
#include "stdio.h"


/*
    SPI register setting value. 
*/
#define SPI_CLOCK_DIV4      0x00
#define SPI_CLOCK_DIV16     0x01
#define SPI_CLOCK_DIV64     0x02
#define SPI_CLOCK_DIV128    0x03
#define SPI_CLOCK_DIV2      0x04
#define SPI_CLOCK_DIV8      0x05
#define SPI_CLOCK_DIV32     0x06

#define SPI_MODE0           0x00
#define SPI_MODE1           0x04
#define SPI_MODE2           0x08
#define SPI_MODE3           0x0C

#define SPI_MODE_MASK       0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define SPI_CLOCK_MASK      0x03  // SPR1 = bit 1, SPR0 = bit 0 on SPCR
#define SPI_2XCLOCK_MASK    0x01  // SPI2X = bit 0 on SPSR

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef	__cplusplus
extern "C" {
#endif

typedef void (*gcSpiHandleRx)(void *p);
typedef void (*gcSpiHandleTx)(void);

extern unsigned char wlan_rx_buffer[];
extern unsigned char wlan_tx_buffer[];


/*
    Arduino pin function setting.
*/
/* This config is for ATMEGA2560 */
#if 0
#define WLAN_CS             24      // Arduino pin connected to CC3000 WLAN_SPI_CS
#define WLAN_EN             25       // Arduino pin connected to CC3000 VBAT_SW_EN
#define WLAN_IRQ            21       // Arduino pin connected to CC3000 WLAN_SPI_IRQ
#define WLAN_IRQ_INTNUM     2       // The attachInterrupt() number that corresponds to WLAN_IRQ
#endif

/* This config is for ATMEGA328P */
#if 1
#define WLAN_CS             4      // Arduino pin connected to CC3000 WLAN_SPI_CS
#define WLAN_EN             7       // Arduino pin connected to CC3000 VBAT_SW_EN
#define WLAN_IRQ            2       // Arduino pin connected to CC3000 WLAN_SPI_IRQ
#define WLAN_IRQ_INTNUM     0       // The attachInterrupt() number that corresponds to WLAN_IRQ
#endif

#define WLAN_MOSI           MOSI
#define WLAN_MISO           MISO
#define WLAN_SCK            SCK

/*
    Arduino SPI hardware setting.
*/
#define WLAN_SPI_MODE       SPI_MODE1
#define WLAN_SPI_BITORDER   MSBFIRST
#define WLAN_SPI_CLOCK_DIV  SPI_CLOCK_DIV4



//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void CC3000_Init(void);

extern void SpiOpen(gcSpiHandleRx pfRxHandler);
extern void SpiClose(void);
extern long SpiWrite(unsigned char *pUserBuffer, unsigned short usLength);
extern void SpiResumeSpi(void);
extern void SpiCleanGPIOISR(void);

extern long ReadWlanInterruptPin(void);
extern void WlanInterruptEnable(void);
extern void WlanInterruptDisable(void);
extern void WriteWlanPin( unsigned char val );

#ifdef	__cplusplus
}
#endif

#endif
