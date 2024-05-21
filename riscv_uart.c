/*
 * riscv_uart.c
 *
 *  Created on: Feb 2, 2021
 *      Author: user
 */
 
#include "riscv_types.h"
#include <riscv_uart.h>
#include <log.h>

//Estructura de datos que permite acceder a los registros de la //UART de LEON3

struct UART_regs
{
	/** \brief UART  Data Register */
	volatile uint32_t Data;   	/* 0xFC001000 */
	/** \brief UART  Status Register */
	volatile uint32_t Status; 	/* 0xFC001004 */
	/** \brief UART  Control Register */
	volatile uint32_t Ctrl; 	/* 0xFC001008 */
	/** \brief UART  Scaler Register */
	volatile uint32_t Scaler; 	/* 0xFC00100C */
};

//********************************************************
//STATUS REGISTER MASKS

//!LEON3 UART DATA READY
#define riscv_UART_DR  (0x01)

//!LEON3 UART A TX FIFO is full
#define riscv_UART_TFF (0x200)

//!LEON3 UART A TX FIFO is empty
#define riscv_UART_TFE  (0x004)

#define LEON_UART_RE    0x00000001 /* RX enable */
#define LEON_UART_TE    0x00000002 /* TX enable */
#define LEON_UART_LB 	0x00000080 /* Loop Back enable */
#define LEON_UART_RI    0x00000004 /* Receiver interrupt enable */

volatile struct UART_regs * const pUART_REGS= (struct UART_regs *)0xFC001000;

//**********************************************************************************
//#define riscv_UART_TF_IS_FULL()

#define riscv_UART_TF_IS_FULL() (riscv_UART_TFF & pUART_REGS->Status)

//**********************************************************************************
//int8_t riscv_putchar(char c)

int8_t riscv_putchar(char c)
{
	uint32_t write_timeout=0;

	while( (riscv_UART_TF_IS_FULL()) && (write_timeout < 0xAAAAA))
	{
		write_timeout++;
	}

	if(write_timeout <  0xAAAAA)
		pUART_REGS->Data=c;

	return (write_timeout ==  0xAAAAA);

return 1;
}

//**********************************************************************************
//int8_t riscv_print_string(char* str)

int8_t riscv_print_string(char* str){

	int8_t error=0;

	while(*str!='\0'&&(!error)){
		error=riscv_putchar(*str);
		//error=riscv_putchar(str[i]);
		str++;
	}

	//Error: Debes añadir la espera a que la fifo de transmisión se vacie
	while(!riscv_uart_tx_fifo_is_empty())
	 ;

	//Debes retornar el error
	//return error;
	return error;
}

//**********************************************************************************
//int8_t riscv_uart_tx_fifo_is_empty()

int8_t riscv_uart_tx_fifo_is_empty() {
	return (riscv_UART_TFE & pUART_REGS->Status);

}

//**********************************************************************************
//int8_t riscv_getchar()

int32_t riscv_getchar() {
	int32_t uart_rx_data;
	
	if ( riscv_UART_DR & pUART_REGS->Status)
	{
		uart_rx_data=pUART_REGS->Data;
	}
	else
	{
		uart_rx_data= -1;
	}

 	return uart_rx_data;
}

//**********************************************************************************
//void riscv_uart_enable_TX()

void riscv_uart_enable_TX()
{
	pUART_REGS->Ctrl |= LEON_UART_TE;
}

//**********************************************************************************
//void riscv_uart_enable_RX()

void riscv_uart_enable_RX()
{
	pUART_REGS->Ctrl |= LEON_UART_RE;
}

//**********************************************************************************
//void riscv_uart_enable_LB()

void riscv_uart_enable_LB()
{
	pUART_REGS->Ctrl |= LEON_UART_LB;
}

//**********************************************************************************
//void riscv_uart_disable_LB()

void riscv_uart_disable_LB()
{
	pUART_REGS->Ctrl &= ~LEON_UART_LB;
}

//**********************************************************************************
//void riscv_uart_enable_RI()

void riscv_uart_enable_RI()
{
	pUART_REGS->Ctrl |= LEON_UART_RI;
}

//**********************************************************************************
//void riscv_uart_disable_RI()

void riscv_uart_disable_RI()
{
	pUART_REGS->Ctrl &= ~LEON_UART_RI;
}

//**********************************************************************************
//void riscv_uart_set_scaler(uint32_t scaler)
//
// scaler value = (SYCLK) / (BaudRate *8)  - 1
void riscv_uart_set_scaler( uint32_t scaler )
{
	pUART_REGS->Scaler = scaler;
}


