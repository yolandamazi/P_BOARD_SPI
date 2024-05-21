/*
 * riscv_monotonic_clock.c
 *
 *  Created on: Mar 15, 2020
 *      Author: user
 */

#include "riscv_types.h"
#include "clinc.h"
#include "log.h"
#include "riscv_monotonic_clock.h"

//*******************************************************************
/** \brief Universal Time Reference in seconds from 1/1/2000 */
static uint32_t UniversalTimeRefY2K;

//*******************************************************************

void InitMonotonicClock( uint32_t current_time )
{
	UniversalTimeRefY2K = current_time;
}


uint32_t GetUniversalTime_Y2K()
{
	uint32_t seconds_from_boot;
	seconds_from_boot = (uint32_t)(get_ticks_from_reset()/CLINT_CLOCK); // WARNING, posible lose of precision
	return (UniversalTimeRefY2K+seconds_from_boot);
}

#define DAYS_TO_1_FEBRUARY					31
#define DAYS_TO_1_MARCH_NO_LEAP_YEAR		(DAYS_TO_1_FEBRUARY + 28)
#define DAYS_TO_1_APRIL_NO_LEAP_YEAR		(DAYS_TO_1_MARCH_NO_LEAP_YEAR +31)
#define DAYS_TO_1_MAY_NO_LEAP_YEAR			(DAYS_TO_1_APRIL_NO_LEAP_YEAR +30)
#define DAYS_TO_1_JUNE_NO_LEAP_YEAR			(DAYS_TO_1_MAY_NO_LEAP_YEAR +31)
#define DAYS_TO_1_JULY_NO_LEAP_YEAR			(DAYS_TO_1_JUNE_NO_LEAP_YEAR +30)
#define DAYS_TO_1_AUGUST_NO_LEAP_YEAR		(DAYS_TO_1_JULY_NO_LEAP_YEAR +31)
#define DAYS_TO_1_SEPTEMBER_NO_LEAP_YEAR	(DAYS_TO_1_AUGUST_NO_LEAP_YEAR +31)
#define DAYS_TO_1_OCTOBER_NO_LEAP_YEAR		(DAYS_TO_1_SEPTEMBER_NO_LEAP_YEAR +30)
#define DAYS_TO_1_NOVEMBER_NO_LEAP_YEAR  	(DAYS_TO_1_OCTOBER_NO_LEAP_YEAR +31)
#define DAYS_TO_1_DECEMBER_NO_LEAP_YEAR  	(DAYS_TO_1_NOVEMBER_NO_LEAP_YEAR +30)

//*******************************************************************

void print_date_time_from_Y2K(uint32_t seconds_from_y2K){

	uint32_t 	aux;

	uint8_t 	second;
	uint8_t 	minute;
	uint8_t 	hour;
    uint16_t 	day;
    uint8_t 	month;

    uint32_t 	year;
    uint8_t 	leap_year=0; //false

    aux=seconds_from_y2K;

    second=aux%60;

    aux=(aux-second)/60; //minutes
    minute=aux%60;

    aux=(aux-minute)/60; //hours
    hour=aux%24;

    aux=(aux-hour)/24; //days

    //Year

    year=aux/365;

    if (year>0){
    	day=aux-year*365;

	    if (day >= ((year-1)/4 +1))
               day=day-((year-1)/4 +1);
        else{
               year--;
               day=day+365-((year-1)/4 +1);
        }

	    if(((year-1)%4)==0)
	    	leap_year=1; // true

    }else {
    	day=aux;
    	leap_year=1; //true
    }

    day++; //first day is 0

    if(day >= (DAYS_TO_1_DECEMBER_NO_LEAP_YEAR + 1*leap_year)){
    	month=12; day=day-(DAYS_TO_1_DECEMBER_NO_LEAP_YEAR + 1*leap_year);
	}else if(day >= (DAYS_TO_1_NOVEMBER_NO_LEAP_YEAR + 1*leap_year)){
		month=11; day=day-(DAYS_TO_1_NOVEMBER_NO_LEAP_YEAR + 1*leap_year);
	}else if(day >= (DAYS_TO_1_OCTOBER_NO_LEAP_YEAR + 1*leap_year)){
		month=10; day=day-(DAYS_TO_1_OCTOBER_NO_LEAP_YEAR + 1*leap_year);
	}else if(day >= (DAYS_TO_1_SEPTEMBER_NO_LEAP_YEAR + 1*leap_year)){
		month=9; day=day-(DAYS_TO_1_SEPTEMBER_NO_LEAP_YEAR + 1*leap_year);
	}else if(day >= (DAYS_TO_1_AUGUST_NO_LEAP_YEAR + 1*leap_year)){
		month=8; day=day-(aux-(DAYS_TO_1_AUGUST_NO_LEAP_YEAR + 1*leap_year));
	}else if(day >= (DAYS_TO_1_JULY_NO_LEAP_YEAR + 1*leap_year)){
		month=7; day=day-(aux-(DAYS_TO_1_JULY_NO_LEAP_YEAR + 1*leap_year));
	}else if(day >= (DAYS_TO_1_JUNE_NO_LEAP_YEAR + 1*leap_year)){
		month=6; day=day-(DAYS_TO_1_JUNE_NO_LEAP_YEAR + 1*leap_year);
	}else if(day >= (DAYS_TO_1_MAY_NO_LEAP_YEAR + 1*leap_year)){
		month=5; day=day-(DAYS_TO_1_MAY_NO_LEAP_YEAR + 1*leap_year);
	}else if(day >= (DAYS_TO_1_APRIL_NO_LEAP_YEAR + 1*leap_year)){
		month=4; day=day-(DAYS_TO_1_APRIL_NO_LEAP_YEAR + 1*leap_year);
	}else if(day >= (DAYS_TO_1_MARCH_NO_LEAP_YEAR + 1*leap_year)){
		month=3; day=day-(DAYS_TO_1_MARCH_NO_LEAP_YEAR + 1*leap_year);
	}else if(day >= (DAYS_TO_1_FEBRUARY )){
		month=2; day=day-(DAYS_TO_1_FEBRUARY);
	}else month=1;

    printf("Fecha: %d | %d | %d    Hora: %d:%d:%d\n", day, month, year+2000, hour, minute, second);
}

//*******************************************************************

uint32_t date_time_to_Y2K(uint8_t day, uint8_t month, uint8_t year
					, uint8_t hour, uint8_t minutes, uint8_t seconds ){

	uint8_t leap_year;

	uint32_t total_days;
	uint32_t y2K;

	total_days=year*365;

	if(year){
		total_days+=(year-1)/4 + 1;
		leap_year= ((year-1)%4==0);
	}else
		leap_year=1; // true

	switch(month){
		   case(2):
				total_days+=DAYS_TO_1_FEBRUARY;break;
	   	   case(3):
				total_days+=DAYS_TO_1_MARCH_NO_LEAP_YEAR+1*leap_year;break;
	   	   case(4):
				total_days+=DAYS_TO_1_APRIL_NO_LEAP_YEAR+1*leap_year;break;
	   	   case(5):
				total_days+=DAYS_TO_1_MAY_NO_LEAP_YEAR+1*leap_year;break;
	   	   case(6):
				total_days+=DAYS_TO_1_JUNE_NO_LEAP_YEAR+1*leap_year;break;
	   	   case(7):
				total_days+=DAYS_TO_1_JULY_NO_LEAP_YEAR+1*leap_year;break;
	   	   case(8):
				total_days+=DAYS_TO_1_AUGUST_NO_LEAP_YEAR+1*leap_year;break;
	   	   case(9):
				total_days+=DAYS_TO_1_SEPTEMBER_NO_LEAP_YEAR+1*leap_year;break;
	   	   case(10):
				total_days+=DAYS_TO_1_OCTOBER_NO_LEAP_YEAR+1*leap_year;break;
	   	   case(11):
				total_days+=DAYS_TO_1_NOVEMBER_NO_LEAP_YEAR+1*leap_year;break;
	   	   case(12):
				total_days+=DAYS_TO_1_DECEMBER_NO_LEAP_YEAR+1*leap_year;break;
	 }void delay( uint64_t ticks );
	 void wait_until( uint64_t ticks_from_reset );


	 total_days+= (day-1);

	 y2K=((total_days*24 + hour)*60 + minutes)*60 + seconds;

	 return y2K;
}

/*
uint64_t get_ticks_from_reset()
{
	return *p_clinc_mtime;
}
*/

void delay( uint64_t ticks_to_wait )
{
	uint64_t target_ticks = get_ticks_from_reset() + ticks_to_wait;
	uint64_t current_ticks;

	do {
		current_ticks = get_ticks_from_reset();
	} while ( current_ticks < target_ticks );
}

void wait_until( uint64_t target_ticks )
{
	uint64_t current_ticks;

	do {
		current_ticks = get_ticks_from_reset();
	} while ( current_ticks < target_ticks );
}

