/*
 * nexys_spi.c
 *
 *  Created on:
 *      Author: user
 */

#include <riscv_types.h>
#include <log.h>
#include "nav_spi.h"

/*
spi0      Cobham Gaisler  SPI Controller    
            APB: fc002000 - fc002100
            IRQ: 5
*/

struct SPI_regs * pNAV_SPI_REGS= (struct SPI_regs *) 0xfc002000;

void printREGS()
{
	printf("----------------------------------\n" );
	printf("Capability0:  %08X\n",  pNAV_SPI_REGS->Capability0 );
	printf("Capability1:  %08X\n",  pNAV_SPI_REGS->Capability1 );
	printf("Mode:         %08X\n",  pNAV_SPI_REGS->Mode );
	printf("Event:        %08X\n",  pNAV_SPI_REGS->Event );
	printf("Mask:         %08X\n",  pNAV_SPI_REGS->Mask );
	printf("Slave Select: %08X\n",  pNAV_SPI_REGS->SlaveSelect );
	printf("----------------------------------\n" );
}

void config_spi_nav(void)
{
	pNAV_SPI_REGS->Mode = 0; 	   //Reset

	pNAV_SPI_REGS->Mode &=~(1<<30);   //LOOP mode disabled
//	pNAV_SPI_REGS->Mode |= (1<<30);   //LOOP mode enabled
	pNAV_SPI_REGS->Mode |=(1<<25);    //Master enabled
	pNAV_SPI_REGS->Mode |=(1<<26);    //REV enabled: MSB transmited first, see TX/RX regs
//	pNAV_SPI_REGS->Mode |=(10<<16);   //PM:3  40Mhz/16 => 2,5 Mhz (SCK)  (ARTY)
	pNAV_SPI_REGS->Mode |=(4<<16);    //PM:3  40Mhz/20 => 2 Mhz (SCK)  (ARTY)
//	pNAV_SPI_REGS->Mode |=(15<<16);   //PM:15  250Mhz/64 => 3,9 Mhz (SCK)
	pNAV_SPI_REGS->Mode |=(1<<2);     //Ignore SPISEL

//	pNAV_SPI_REGS->Mode &=~(0xF<<20); //LEN = 0  (WLEN = 32 bits)
	pNAV_SPI_REGS->Mode |= (0xF<<20); //LEN = 15 (WLEN = 16 bits)
//	pNAV_SPI_REGS->Mode |= (0x7<<20); //LEN = 7  (WLEN = 8 bits)

	pNAV_SPI_REGS->Mode |=(1<<24); //Core enabled

	printREGS();
}

uint8_t read_nav(uint8_t slave, uint8_t reg_id)
{
	uint32_t write_timeout=0;
	uint32_t word;

	pNAV_SPI_REGS->Event |= (0x3<<11); //Clean flags UN and OV
	while(((pNAV_SPI_REGS->Event & (1<<8))==0) && (write_timeout < 0xAAAAA))
	{
		write_timeout++; //Wait until NF=1
	}

	if(write_timeout < 0xAAAAA)
	{
		// RW: 1 -> Read, 0 -> Write 
		// TX request: <RW:1bit><REG_ID:7bits>    :     <0>  
		// RX data:                               <REG Value:8bits>  
   
		word=0;
		word |= ((0x80 | reg_id) << 24);	 // REG_ID, Read
		 
		pNAV_SPI_REGS->SlaveSelect &= ~(1 << slave); // Slave select goes down 

		pNAV_SPI_REGS->Transmit=word; //Transmit word

		while(pNAV_SPI_REGS->Event & (1<<31))
			; // Wait while transfer in progress 

		pNAV_SPI_REGS->SlaveSelect |= (1 << slave); // Slave select goes up 

		// Read received data
		if (pNAV_SPI_REGS->Event & (1<<9))
		{
			// RX register 16 MSB bits contains REG value and REG+1 value 
			// printf("DATA: %08X\n", pNAV_SPI_REGS->Receive );
			return (pNAV_SPI_REGS->Receive >> 16);
		}
		else
		{
			printf("Error: no data received...\n");
		}
	}
	else
	{
		printf("Read reg NF Timeout...\n");
	}

	return 0xFF;
}

void write_nav(uint8_t reg_id, uint8_t value)
{
	uint32_t write_timeout=0, word;
	uint8_t scratch __attribute__((unused));

	pNAV_SPI_REGS->Event |= (0x3<<11); //Clean flags UN and OV
	while(((pNAV_SPI_REGS->Event & (1<<8))==0) && (write_timeout < 0xAAAAA))
	{
		write_timeout++; //Wait until NF=1
	}

	if(write_timeout < 0xAAAAA)
	{
		// RW: 1 -> Read, 0 -> Write 
		// TX request: <RW:1bit><REG_ID:7bits>  : <TX data:8bits>
   
		word=0;
		word |= ((reg_id) << 24);	 // REG_ID, Write
		word |= ((value) << 16);	 

		pNAV_SPI_REGS->SlaveSelect &= ~(1 << 0); // Slave select 0 goes down 

		pNAV_SPI_REGS->Transmit=word; //Transmit word

		while(pNAV_SPI_REGS->Event & (1<<31))
			; // Wait while transfer in progress 

		pNAV_SPI_REGS->SlaveSelect |= (1 << 0); // Slave select 0 goes up 

		// Clean received buffer
		while (pNAV_SPI_REGS->Event & (1<<9))
		{
			scratch = pNAV_SPI_REGS->Receive;
		}
	}
	else
	{
		printf("Write reg NF Timeout...\n");
	}

	return;
}

void config_nav_measurement(void)
{
	//enable all three axes
	write_nav(CTRL_REG5_XL, 0x38);
	
	//set 10Hz odr for accelerometer, +/-2g range
	write_nav(CTRL_REG6_XL, 0x20);
}

uint16_t getACEL_X()
{
	uint8_t AclX_L, AclX_H;

	AclX_L=read_nav( ACCELEROMETER_SLAVE, OUT_X_L_XL );
	AclX_H=read_nav( ACCELEROMETER_SLAVE, OUT_X_H_XL );
	return ( (int16_t)(AclX_H << 8) | AclX_L );

}

uint16_t getACEL_Y()
{
	uint8_t AclY_L, AclY_H;

	AclY_L=read_nav( ACCELEROMETER_SLAVE, OUT_Y_L_XL );
	AclY_H=read_nav( ACCELEROMETER_SLAVE, OUT_Y_H_XL );
	return ( (int16_t)(AclY_H << 8) | AclY_L );
}

uint16_t getACEL_Z()
{
	uint8_t AclZ_L, AclZ_H;

	AclZ_L=read_nav( ACCELEROMETER_SLAVE, OUT_Z_L_XL );
	AclZ_H=read_nav( ACCELEROMETER_SLAVE, OUT_Z_H_XL );
	return ( (int16_t)(AclZ_H << 8) | AclZ_L );
}


