# RISC-V baremetal init.s
# This code is executed first.

.section .text.init
entry:
    la    t0, __mem    #Load base address into t0	definido en link.ld, 1 Mbyte de memoria
    la    t1, __stack  #Load stack offset into t1       en el comando de ejecución de SPIKE se reserva
    add   sp, t0, t1   #Add the memory base and the stack offset and set it as the stack pointer.

    call startup       # 

    call  main         #call the main function

    la      t1, tohost 
    li      t2, 1
    sw      t2, 0(t1)
end:
    ebreak
#    j end               #wait for enviroment to get control.

#.globl trap		# TODO: tratamiento de traps
#trap:
#    #add  a7, x0, a0
#    ecall
#    ret

#.section .tohost, "aw", @progbits
.section .data
.globl tohost
.align 4
tohost: .dword 0
.globl fromhost
.align 4
fromhost: .dword 0
