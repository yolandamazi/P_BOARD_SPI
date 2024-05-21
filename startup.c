#include <riscv_types.h>
#include "riscv-csr.h"
#include "plic.h"
#include "clinc.h"
#include "dispatch.h"

#include "log.h"

// Machine mode software interrutp
volatile uint32_t *p_clinc_msip    = (uint32_t *)(CLINC_BASE + MSIP_BASE);
// Local timer compare value
volatile uint64_t *p_clinc_timecmp = (uint64_t *)(CLINC_BASE + MTIMECMP_BASE);
// Local current timer value
volatile uint64_t *p_clinc_mtime   = (uint64_t *)(CLINC_BASE + MTIME_BASE);

struct plic_regs *p_plic = (struct plic_regs *)(PLIC_BASE);
struct plic_context_regs *p_plic_context = (struct plic_context_regs *)(PLIC_BASE+PLIC_CONTEXT);

//
// Bare IRQ/EXCEPTION handling routine
// It's address is saved in MTVEC during startup
// 
__attribute__ ((interrupt ())) void bare_isr_handle ()
{
uint32_t mcause, code, trap, param;

// Get register A0 value, in case of syscall (executive call) containes trap value
register uint32_t a5 asm ("a5") = (uint32_t)&trap;
asm volatile("sw a0, 0(a5)" : : "r" (a5) );
// Get register A1 value, in case of syscall (executive call) containes param value
register uint32_t a6 asm ("a6") = (uint32_t)&param;
asm volatile("sw a1, 0(a6)" : : "r" (a6) );

// Get mcause, interrupt or exception

/*
Interrupt Exception Code Description
MSB	CODE
1	0 User software interrupt
1	1 Supervisor software interrupt
1	2 Hypervisor software interrupt
1	3 Machine software interrupt
1	4 User timer interrupt
1	5 Supervisor timer interrupt
1	6 Hypervisor timer interrupt
1	7 Machine timer interrupt
1	8 User external interrupt
1	9 Supervisor external interrupt
1	10 Hypervisor external interrupt
1	11 Machine external interrupt
1	≥12 Reserved
0	0 Instruction address misaligned
0	1 Instruction access fault
0	2 Illegal instruction
0	3 Breakpoint
0	4 Load address misaligned
0	5 Load access fault
0	6 Store/AMO address misaligned
0	7 Store/AMO access fault
0	8 Environment call from U-mode
0	9 Environment call from S-mode
0	10 Environment call from H-mode
0	11 Environment call from M-mode
0	≥12 Reserved
*/

   mcause = csr_read_mcause();

   code = mcause & 0x7FFFFFFF;

   if ( mcause & 0x80000000 ) // MSB test: 1 IRQ / 0 Exception
   {
	// Dispatch IRQ
	   irq_dispatch(code);
   }
   else
   {
	 if ( code == RISCV_MACHINE_CALL_M_MODE )
	 {
		// Dispatch TRAP: Executive call
		syscall_dispatch(trap, param);
	 }
	 else
	 {
		// Dispatch exception
		excepcion_dispatch(code);
	 }

	 // FIX mret: return from trap
	 asm volatile("csrr t0, mepc");
	 asm volatile("addi t0, t0, 0x4");
	 asm volatile("csrw mepc, t0");
   }

   return;
}

void raise_SW_interrupt( uint32_t hart )
{
	// TO COMPLETE
}


// SET STATUS REG MIE BIT (Global IRQ enable)
void enable_irq(void)
{
uint32_t val;

	val = csr_read_mstatus();
	val |= MSTATUS_MIE_BIT_MASK;
	csr_write_mstatus( val );
}

// CLEAR STATUS REG MIE BIT (Global IRQ disable)
void disable_irq(void)
{
uint32_t val;

	val = csr_read_mstatus();
	val &= ~MSTATUS_MIE_BIT_MASK;
	csr_write_mstatus( val );
}

// SET MIE REG EXT_IRQ BIT (EXT IRQ enable)
void enable_ext_irq(void)
{
uint32_t val;

	val = csr_read_mie();
	val |= MIE_MEI_BIT_MASK;
	csr_write_mie( val );
}

// CLEAR MIE REG EXT_IRQ BIT (EXT IRQ disable)
void disable_ext_irq(void)
{
uint32_t val;

	val = csr_read_mie();
	val &= ~MIE_MEI_BIT_MASK;
	csr_write_mie( val );
}

// SET MIE REG TIMER BIT (Timer irq enable)
void enable_timer_clinc_irq(void)
{
uint32_t val;

	*p_clinc_mtime = 0;

	val = csr_read_mie();
	val |= MIE_MTI_BIT_MASK;
	csr_write_mie( val );
}

// CLEAR MIE REG TIMER BIT (Timer irq disable)
void disable_timer_clinc_irq(void)
{
uint32_t val;

	val = csr_read_mie();
	val &= ~MIE_MTI_BIT_MASK;
	csr_write_mie( val );
}

// SET MIE REG SW BIT
void enable_SW_IRQ(void)
{
uint32_t val;

	val = csr_read_mie();
	val |= MIE_MSI_BIT_MASK;
	csr_write_mie( val );
}

// CLEAR MIE REG SW BIT
void disable_SW_IRQ(void)
{
uint32_t val;

	val = csr_read_mie();
	val &= ~MIE_MSI_BIT_MASK;
	csr_write_mie( val );
}

// MASK PLIC external IRQ
void plic_irq_mask(uint32_t source)
{
        volatile uint32_t *maskreg;
        maskreg = &p_plic->enable[0][source / 32];
        uint32_t keepbits = ~(1 << source);
    //int plevel;

	//disable_ext_irq();
        *maskreg &= keepbits;
	//enable_ext_irq();

        return;
}

// UNMASK PLIC external IRQ
void plic_irq_unmask(uint32_t source)
{
        volatile uint32_t *maskreg;
        maskreg = &p_plic->enable[0][source / 32];
        uint32_t setbit = (1 << source);
    //int plevel;

	//disable_ext_irq();
        *maskreg |= setbit;
	//enable_ext_irq();

        return;
}

void init_plic( void )
{
	// threshold value of zero enables all interrupts with non-zero priority
	// whereas a value of 7 masks all interrupts.
    p_plic_context[0].priority_threshold = 0;

    for (int i = 1; i < 32; i++)
    {
	   p_plic->priority[i] = 1; // Sets the same priority for all interrupts
	   plic_irq_mask( i );
    }
}

//
// Interrupt management configuration, called from init.S
//
void startup()
{
    disable_irq();	    // Global interrupt disable (MSSATUS)
    csr_write_mie(0);	// ZERO MIE, all interrupts disabled

    // Setup the IRQ handler entry point
    csr_write_mtvec((uint32_t) bare_isr_handle);

    init_plic();

    return;
}
