################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libconfini/src/confini.c 

OBJS += \
./libconfini/src/confini.o 

C_DEPS += \
./libconfini/src/confini.d 


# Each subdirectory must supply rules for building sources it contributes
libconfini/src/%.o: ../libconfini/src/%.c libconfini/src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"../hpsdr/include" -I"../librpitx-C/librpitx" -I"../librpitx-C/librpitx/core/include" -I"../librpitx-C/librpitx/modulation/include" -I"../exceptions4c/src" -I"../libconfini/src" -O3 -g3 -Wall -c -fmessage-length=0 -fcommon -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


