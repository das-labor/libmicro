#ifndef UART_H
#define UART_H

/**
 * UART Library
 *
 * #define F_CPU 16000000         // Oszillator-Frequenz in Hz
 * #define UART_INTERRUPT 1
 * #define UART_BAUD_RATE 19200
 * #define UART_RXBUFSIZE 16
 * #define UART_TXBUFSIZE 16
 * #define UART_LINE_BUFFER_SIZE 40
 * #define UART_LEDS             // LED1 and LED2 toggle on tx and rx interrupt
 *
 */
#ifdef __AVR__
#include <inttypes.h>
#include <avr/pgmspace.h>

void uart_init();
void uart_putstr_P(PGM_P str);
void uart_hexdump(char *buf, int len);

//get one Cariage return terminated line
//echo charakters back on Uart
//returns buffer with zero terminated line on success, 0 pointer otherwise
char *uart_getline_nb();

#else
#include "config.h"
#include "debug.h"
#include <sys/select.h>

void uart_init(char *sport);
void uart_close(void);

/* UART-Host specific */
int uart_fd;

#endif

void uart_putc(char c);
void uart_putstr(char *str);
char uart_getc(void);
char uart_getc_nb(char *c);

#endif
