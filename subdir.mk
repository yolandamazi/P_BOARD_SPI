################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../startup/dispatch.c \
../startup/startup.c 

S_UPPER_SRCS += \
../startup/init.S 

C_DEPS += \
./startup/dispatch.d \
./startup/startup.d 

OBJS += \
./startup/dispatch.o \
./startup/init.o \
./startup/startup.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.c startup/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	riscv-gaisler-elf-gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

startup/%.o: ../startup/%.S startup/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Assembler'
	riscv-gaisler-elf-as  -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-startup

clean-startup:
	-$(RM) ./startup/dispatch.d ./startup/dispatch.o ./startup/init.o ./startup/startup.d ./startup/startup.o

.PHONY: clean-startup

