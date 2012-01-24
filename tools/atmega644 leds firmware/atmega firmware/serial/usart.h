#pragma once
#include <inttypes.h>

uint8_t FIFO_tx_slip_p(uint8_t leng,uint8_t* from_pnt,uint8_t wait);
uint8_t FIFO_rx_slip_p(uint8_t leng,uint8_t* to_pnt);

void uart_init(void) ;

// void uart_putc(uint8_t c) ;
//
// void uart_puts(const char* c) ;

static void uart_enable_tx_interrupt(void);
static void uart_disable_tx_interrupt(void);