/*****************************************************************************
*
*  l_debug.c  - Low level simple UART output driver for driver debugging.
*
*****************************************************************************/
#include "l_debug.h"

void ldbg_init(unsigned long baud) {

    pinMode(1, OUTPUT);
    pinMode(0, INPUT);
    delayMicroseconds(50);
    
    uint16_t baud_setting;
    int use_u2x = 1;

#if F_CPU == 16000000UL
    // hardcoded exception for compatibility with the bootloader shipped
    // with the Duemilanove and previous boards and the firmware on the 8U2
    // on the Uno and Mega 2560.
    if (baud == 57600) {
        use_u2x = 0;
    }
#endif

try_again:
  
    if (use_u2x) {
        UCSR0A = 1 << U2X0;
        baud_setting = (F_CPU / 4 / baud - 1) / 2;
    } else {
        UCSR0A = 0;
        baud_setting = (F_CPU / 8 / baud - 1) / 2;
    }

    if ((baud_setting > 4095) && use_u2x)
    {
        use_u2x = false;
        goto try_again;
    }

    // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
    UBRR0H = baud_setting >> 8;
    UBRR0L = baud_setting;

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

void ldbg_putchar(char c) {
    while( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = c;
}

void ldbg_putmem(char* mem, unsigned int size){
    unsigned int i;
    for(i = 0; i < size; i++){
        ldbg_putchar(mem[i]);
    }
}

char ldbg_getchar(void) {
    while( !(UCSR0A & (1<<RXC0)) );
    return UDR0;
}