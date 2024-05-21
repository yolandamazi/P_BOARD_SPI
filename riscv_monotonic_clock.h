#ifndef RISCV_MONOTONIC_H_
#define RISCV_MONOTONIC_H_

// #define CLINT_CLOCK (10000000)  // SPIKE
#define CLINT_CLOCK (40000000)  // NOEL Board

void InitMonotonicClock( uint32_t current_time );

uint32_t GetUniversalTime_Y2K();

void print_date_time_from_Y2K(uint32_t seconds_from_y2K);

uint32_t date_time_to_Y2K( uint8_t day, uint8_t month, uint8_t year,
						   uint8_t hour, uint8_t minutes, uint8_t seconds );

// uint64_t get_ticks_from_reset();
#define get_ticks_from_reset() ((volatile uint64_t)(*p_clinc_mtime))

void delay( uint64_t ticks_to_wait );
void wait_until( uint64_t ticks_from_reset );

#endif /* RISCV_MONOTONIC_H_*/
