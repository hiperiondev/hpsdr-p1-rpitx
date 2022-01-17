# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../librpitx-C/librpitx/core/dma.c \
../librpitx-C/librpitx/core/dsp.c \
../librpitx-C/librpitx/core/gpio.c \
../librpitx-C/librpitx/core/mailbox.c \
../librpitx-C/librpitx/core/raspberry_pi_revision.c \
../librpitx-C/librpitx/core/rpi.c \
../librpitx-C/librpitx/core/util.c 

OBJS += \
./librpitx-C/librpitx/core/dma.o \
./librpitx-C/librpitx/core/dsp.o \
./librpitx-C/librpitx/core/gpio.o \
./librpitx-C/librpitx/core/mailbox.o \
./librpitx-C/librpitx/core/raspberry_pi_revision.o \
./librpitx-C/librpitx/core/rpi.o \
./librpitx-C/librpitx/core/util.o 

C_DEPS += \
./librpitx-C/librpitx/core/dma.d \
./librpitx-C/librpitx/core/dsp.d \
./librpitx-C/librpitx/core/gpio.d \
./librpitx-C/librpitx/core/mailbox.d \
./librpitx-C/librpitx/core/raspberry_pi_revision.d \
./librpitx-C/librpitx/core/rpi.d \
./librpitx-C/librpitx/core/util.d 


# Each subdirectory must supply rules for building sources it contributes
librpitx-C/librpitx/core/%.o: ../librpitx-C/librpitx/core/%.c librpitx-C/librpitx/core/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"../hpsdr/include" -I"../librpitx-C/librpitx" -I"../librpitx-C/librpitx/core/include" -I"../librpitx-C/librpitx/modulation/include" -O3 -g3 -Wall -c -fmessage-length=0 -fcommon -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


