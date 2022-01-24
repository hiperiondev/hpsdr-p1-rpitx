################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../exceptions4c/src/e4c.c 

OBJS += \
./exceptions4c/src/e4c.o 

C_DEPS += \
./exceptions4c/src/e4c.d 


# Each subdirectory must supply rules for building sources it contributes
exceptions4c/src/%.o: ../exceptions4c/src/%.c exceptions4c/src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"../hpsdr/include" -I"../librpitx-C/librpitx" -I"../librpitx-C/librpitx/core/include" -I"../librpitx-C/librpitx/modulation/include" -I"../exceptions4c/src" -I"../libconfini/src" -O3 -g3 -Wall -c -fmessage-length=0 -fcommon -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


