

//	FIFO Buffer Defines
//====================================================
//	Val		Buffer Size
//
//	0		2
//	1		4
//	2		8
//	3		16
//	4		32
//	5		64
//	6		128
//	7		256
//
#define FIFO_RX_BUFFER_BITS 4     /* 2,4,8,16,32,64,128 or 256 bytes */
#define FIFO_TX_BUFFER_BITS 6     /* 2,4,8,16,32,64,128 or 256 bytes */
//====================================================


//====================================================
// do not change !!!	do not change	!!!do not change

// the software expects the buffer size to be a multiple of 2
#define FIFO_RX_BUFS (1 << (7 & (1+FIFO_RX_BUFFER_BITS)))
#define FIFO_TX_BUFS (1 << (7 & (1+FIFO_TX_BUFFER_BITS)))
#define FIFO_RX_BUFFER_MASK ( FIFO_RX_BUFS - 1 )
#define FIFO_TX_BUFFER_MASK ( FIFO_TX_BUFS - 1 )

// do not change upto here.  do not change upto here.
//====================================================

// Based on ap note AVR306 by ATMEL
// Routines for interrupt controlled SLIP ONLY FIFO

// fw version 20060124

// Changed : 18 Jan 2006
// changed filename to indicate fifo control
// moved usart driver to another file
// added link to external slip interface
// fixed buffer pointert bug

// Changed : 11 Jan 2006
// added a pointer write routine
// changed buffer pointer access
// fixed buffer pointert bug

// Changed : 6 December 2005
// added uppercase and lowercase receive routines

// Changed : 3 Nov 2005
// changed rx buffer access, added checks for non-atomic interrupt pointer modification
// changed function name DataInReceiveBuffer to FIFO_rx_waiting

// modified: 10 APR 2005
// added double speed baud support;
// Modified by: Murray Horn

// this file includes the tx and rx routines for a single uart mega avr.
// seperate tx and rx ques are available and can be sized in the .inc file
// a string tx feature from rom has been added (uart_puts_p).

//	Includes
#include <inttypes.h>
#include <avr/pgmspace.h>	// used for the string printing feature
#include <avr/interrupt.h>
#include "config.h"
#include <util/setbaud.h>
#include "usart.h"

// Static Variables
static uint8_t FIFO_RxBuf[FIFO_RX_BUFS];
static uint8_t FIFO_TxBuf[FIFO_TX_BUFS];
static volatile uint8_t RX_SLIP_peek;

static volatile uint8_t FIFO_RxHead;
static volatile uint8_t FIFO_RxTail[3];

static volatile uint8_t FIFO_TxHead[3];
static volatile uint8_t FIFO_TxTail;

// pointer based slip implementation for fifo.c
// fw version 20060124

// available from http://www.opend.co.za/
// Written by: Murray Horn
// Created : 23 January 2006
// Compiled on WinAVR 20040404




#define	SLIP_end_char		192 /*0xc0*/
#define	SLIP_data_end_char	220	/*0xdc*/
#define	SLIP_data_esc_char	221 /*0xdd*/
#define	SLIP_esc_char		219 /*0xdb*/

//--------------------------------------------------
// this should be set to ensure the poll time of the rx fifo does not allow for an rx overflow
#define		MAX_SLIP_VOL 	(FIFO_RX_BUFFER_MASK - 5)

/*
// simple debug tools
void rx_debug(void)
{
	uint8_t i;
	uint8_t tmptail,tmphead;

	tmptail = FIFO_RxTail[0];

//----------------------------
// non inter-corruptable interface
	do	tmphead = FIFO_RxHead;
	while (tmphead != FIFO_RxHead);
//----------------------------

	for (i=0;i<32;i++)
		if (tmptail == i)	FIFO_tx_byte('T');
		else				FIFO_tx_byte('.');
	FIFO_tx_s_prg_l(PSTR(""));
	for (i=0;i<32;i++)
		if (RX_SLIP_peek == i)	FIFO_tx_byte('S');
		else					FIFO_tx_byte('.');
	FIFO_tx_s_prg_l(PSTR(""));
	for (i=0;i<32;i++)
		if (tmphead == i)	FIFO_tx_byte('H');
		else				FIFO_tx_byte('.');
	FIFO_tx_s_prg_l(PSTR(""));
	for (i=0;i<32;i++)
		FIFO_tx_byte(FIFO_RxBuf[i]);
	FIFO_tx_s_prg_l(PSTR(""));
}
*/


//-----------------------------------------------------
uint8_t FIFO_rx_slip_p(uint8_t	leng,uint8_t*	to_pnt)
// this is a SLIP receive routine
// the received data is moved to the to_pnt location
// the maximum length is leng (the available space of the pointer location)
// the return value indicates the number of bytes received
// tested
// the volume the SLIP data is allowed to use in the buffer is MAX_SLIP_VOL in fifo.inc
// once this point has been reached, and no valid packet exists, the system starts discarding SLIP packets
{
	uint8_t tmptail,tmphead,nxt_tail;
	uint8_t esc_val,b;
	uint8_t total_leng;
	uint8_t rx_leng;

	while (1)
	{
	
		tmptail = FIFO_RxTail[0];

//----------------------------
// non inter-corruptable interface
		do	tmphead = FIFO_RxHead;
		while (tmphead != FIFO_RxHead);
//----------------------------

	// seek for a start char
		while ((FIFO_RxBuf[tmptail] != SLIP_end_char) && (tmphead != tmptail))
			tmptail = (tmptail + 1) & FIFO_RX_BUFFER_MASK;
	
//----------------------------
// prevent slip_peek from being left behind
			if (tmptail <= FIFO_RxHead)
			{
				if ((tmptail > RX_SLIP_peek) || (RX_SLIP_peek >= FIFO_RxHead))
					RX_SLIP_peek = tmptail;
			}		
			else if ((tmptail > RX_SLIP_peek) && (RX_SLIP_peek >= FIFO_RxHead))
					RX_SLIP_peek = tmptail;
//----------------------------


		FIFO_RxTail[0] = tmptail;
		FIFO_RxTail[1] = tmptail;
		FIFO_RxTail[2] = tmptail;
	
		if (tmphead == tmptail)		return(0);
	
	// ensure sepperate start and end chars
		if (tmptail == RX_SLIP_peek)
			RX_SLIP_peek = (RX_SLIP_peek + 1) & FIFO_RX_BUFFER_MASK;
			
	//	check if an end char came in
		while ((FIFO_RxBuf[RX_SLIP_peek] != SLIP_end_char) && (RX_SLIP_peek != tmphead))
			RX_SLIP_peek = (RX_SLIP_peek + 1) & FIFO_RX_BUFFER_MASK;	

// now we know that peek and tail are unique so the total rx data is at least 2 bytes.

// calculate the slip length
		if ( RX_SLIP_peek >= tmptail)
			rx_leng = RX_SLIP_peek - tmptail;
		else
			rx_leng = FIFO_RX_BUFFER_MASK + 1 + RX_SLIP_peek - tmptail;

// now protect against massive SLIP packets
		if ((rx_leng > MAX_SLIP_VOL) && (FIFO_RxBuf[RX_SLIP_peek] != SLIP_end_char))
			tmptail = (tmptail + 1) & FIFO_RX_BUFFER_MASK;
		else {
// data processing
			if (tmphead == RX_SLIP_peek)	// only check here as the max slip vol has to be checked first
				return(0); 				// the last byte was not yet valid
			
			nxt_tail = (tmptail + 1) & FIFO_RX_BUFFER_MASK;
			total_leng = 0;
			esc_val = 0;
			while (total_leng <= leng)
			{
				tmptail = nxt_tail;
				b = FIFO_RxBuf[tmptail];
				if (1 == esc_val)
				{
					total_leng++;
					esc_val = 0;
					switch(b) {
						case SLIP_data_esc_char : 	*to_pnt++ = SLIP_esc_char;
						break;	
						case SLIP_data_end_char : 	*to_pnt++ = SLIP_end_char;
						break;	
						default :					total_leng = leng; // a error code
						break;
					}
				}
				else
				{
					switch(b) {
						case SLIP_esc_char : 	esc_val = 1;
						break;	
						case SLIP_end_char :
		// save the end location					
							FIFO_RxTail[0] = tmptail;
							FIFO_RxTail[1] = tmptail;
							FIFO_RxTail[2] = tmptail;
							return(total_leng); // accept the packet
						break;	
						default :
							total_leng++;
							*to_pnt++ = b;
						break;
					}
				}
				nxt_tail = (tmptail + 1) & FIFO_RX_BUFFER_MASK;
			}
		}
		// save the end location					
		FIFO_RxTail[0] = tmptail;
		FIFO_RxTail[1] = tmptail;
		FIFO_RxTail[2] = tmptail;
	}
	
}
//-----------------------------------------------------

//-----------------------------------------------------
// this routine moves a packet of data to the tx que in SLIP format
// if wait = 1 the routine will loop untill all the data is queued
// tested
// wait function untested
// a return codes
// 0 : no problems
// 1 : not enough memory
//
uint8_t FIFO_tx_slip_p(uint8_t	leng,uint8_t*	from_pnt,uint8_t	wait)
{
	uint8_t tmptail,tmphead;
	uint8_t *to_pnt;
	uint8_t *tmp_pnt;
	uint8_t leng_av,leng_togo;

	uint8_t i;
	uint8_t b;
	uint8_t esc_val;
	uint8_t	first_time;

//---------------------------
// if there was no data to send return
// not required but a optimization
	if (0 == leng)
	return(0); // no errors
//---------------------------

	first_time = 1;
	esc_val = 0;

//---------------
// calculate the real size required, with slip char
	tmp_pnt = from_pnt;
	i = leng;
	while (i--)
	{
		if (SLIP_end_char == *tmp_pnt)  leng++;
		if (SLIP_esc_char == *tmp_pnt)  leng++;
		tmp_pnt++;
	}
	leng += 2;
//---------------


	tmphead = FIFO_TxHead[0];
	
	do	tmptail = FIFO_TxTail;
	while (tmptail != FIFO_TxTail);		// prevent any interrupt interfearence

// find the available space in the circular buffer
// leng1 is from the head on leng2 is from the base of the cicular buffer (wrap around)
	if ( tmphead < tmptail )		leng_av = tmptail - tmphead -1;
	else							leng_av = FIFO_TX_BUFFER_MASK + tmptail - tmphead ;

// now if there is space, continue
	if ((leng_av < leng) && (0 == wait))
		return(1); // too little memory error
	else
	while (leng > 0)
	{

//------------------------------
// get the maxlength of the sequential tx
		if ( tmphead < tmptail )
			leng_togo = tmptail - tmphead -1;
		else
		{
			if (0 == tmptail)	leng_togo = FIFO_TX_BUFFER_MASK - tmphead;
			else				leng_togo = FIFO_TX_BUFFER_MASK - tmphead+1;
		}	
//------------------------------

// leng_togo now becomes length to be copied
		if (leng_togo > leng)
			leng_togo = leng;

		leng -= leng_togo;
		
		to_pnt = &FIFO_TxBuf[tmphead];
		tmphead = ( tmphead + leng_togo) & FIFO_TX_BUFFER_MASK;


		if ((0 != first_time) && (0 != leng_togo))
		{
			*to_pnt++ = SLIP_end_char;
			leng_togo--;
			first_time = 0;
		}


//---------------------
//---------------------
		if (leng_togo > 0)
		{
//---------------
// for the last char, a special case
			if (0 == leng)
				leng_togo--;
//---------------
			
// save from the head on
			while (leng_togo--)
			{

				if (esc_val)
				{
					*to_pnt++ = esc_val;
					esc_val = 0;
				}
				else
				{
					b = *from_pnt++;
					switch(b) {
						case SLIP_end_char : 
							*to_pnt++ = SLIP_esc_char;
							esc_val = SLIP_data_end_char;
						break;
						case SLIP_esc_char : 
							*to_pnt++ = SLIP_esc_char;
							esc_val = SLIP_data_esc_char;
						break;	
						default :
							*to_pnt++ = b;
						break;
					}	
				}	
			}

//---------------------
// the last char
			if (0 == leng)
				*to_pnt++ = SLIP_end_char;
//---------------------
		}
//---------------------
//---------------------


// save the circular buffer entry
//----------------------------
// non inter-corruptable interface
		FIFO_TxHead[0] = tmphead;
		FIFO_TxHead[1] = tmphead;
		FIFO_TxHead[2] = tmphead;
//----------------------------

// enable the interrupt
		uart_enable_tx_interrupt();

//------------------------------
// re-read for the loop
		do	tmptail = FIFO_TxTail;
		while (tmptail != FIFO_TxTail);		// prevent any interrupt interfearence

	}
	return(0); // no errors
}
//-----------------------------------------------------


//---------------------------

//static volatile uint16_t	USART_overflow;

static void uart_enable_tx_interrupt(void) {
    UCSR0B |= (1<<UDRIE0);
}

static void uart_disable_tx_interrupt(void) {
    UCSR0B &= ~(1<<UDRIE0);
}


void uart_init(void)
{
    UBRR0H    = UBRRH_VALUE;
    UBRR0L    = UBRRL_VALUE;

    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);    // Asynchron 8N1
    //UCSR0C = (1<<UMSEL01)|(1<<UMSEL00) | (1 << UCSZ01) | (1 << UCSZ00);
    //UCSR0C = (1 << USBS1)|(1<<UCSZ1) | (1<<UCSZ0);
    UCSR0B |= (1<<RXCIE0)|(1<<UDRIE0); // RX-Interrupt enable, Data Register Empty Interrupt Enable // TX-Interrupt enable
    UCSR0B |= (1<<TXEN0)|(1<<RXEN0); // RX enable, TX enable

    do
    {
        UDR0;
    }
    while (UCSR0A & (1 << RXC0));

    FIFO_RxTail[0] = 0;
    FIFO_RxTail[1] = 0;
    FIFO_RxTail[2] = 0;
    FIFO_RxHead = 0;
    RX_SLIP_peek = 0;
    FIFO_TxTail = 0;
    FIFO_TxHead[0] = 0;
    FIFO_TxHead[1] = 0;
    FIFO_TxHead[2] = 0;
}


//-----------------------------------------------------
//	RX Interrupt handler
ISR(USART0_RX_vect)
{
    uint8_t data;
    uint8_t tmphead,tmptail;
    uint8_t nxthead;

//	Read the received data
    data = UDR0;
//	Calculate buffer index
    tmphead = FIFO_RxHead;
    nxthead = ( tmphead + 1 ) & FIFO_RX_BUFFER_MASK;

//----------------------------
// non inter-corruptable interface
    tmptail = FIFO_RxTail[1];
    if (tmptail != FIFO_RxTail[2])
        tmptail = FIFO_RxTail[0];
//----------------------------

    if ( nxthead == tmptail )
    {
// ERROR! Receive buffer overflow
        //USART_overflow += 1;
    }
    else
    {
        FIFO_RxBuf[tmphead] = data; // Store received data in buffer
        FIFO_RxHead = nxthead;      // Store new index
    }
}
//-----------------------------------------------------


//-----------------------------------------------------
//	TX Interrupt handler
ISR(USART0_UDRE_vect)
{
    uint8_t	tmphead,tmptail;

//----------------------
// preventing reading the tail while it's being manipulated
    tmphead = FIFO_TxHead[1];
    if (tmphead != FIFO_TxHead[2])
        tmphead = FIFO_TxHead[0];
//----------------------

    tmptail = FIFO_TxTail;

//	Check if all data is transmitted
    if (tmphead != tmptail)
    {
//	Calculate buffer index
        UDR0 = FIFO_TxBuf[tmptail];	// Start transmition
        tmptail = ( tmptail + 1 ) & FIFO_TX_BUFFER_MASK;
        FIFO_TxTail = tmptail;
    }
    else
    {
        uart_disable_tx_interrupt();
    }
}
//-----------------------------------------------------
