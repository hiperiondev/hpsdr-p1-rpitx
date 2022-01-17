# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hpsdr/hpsdr_debug.c \
../hpsdr/hpsdr_ep2.c \
../hpsdr/hpsdr_ep6.c \
../hpsdr/hpsdr_functions.c \
../hpsdr/hpsdr_iq_tx.c \
../hpsdr/hpsdr_main.c \
../hpsdr/hpsdr_network.c \
../hpsdr/hpsdr_tx_samples.c 

OBJS += \
./hpsdr/hpsdr_debug.o \
./hpsdr/hpsdr_ep2.o \
./hpsdr/hpsdr_ep6.o \
./hpsdr/hpsdr_functions.o \
./hpsdr/hpsdr_iq_tx.o \
./hpsdr/hpsdr_main.o \
./hpsdr/hpsdr_network.o \
./hpsdr/hpsdr_tx_samples.o 

C_DEPS += \
./hpsdr/hpsdr_debug.d \
./hpsdr/hpsdr_ep2.d \
./hpsdr/hpsdr_ep6.d \
./hpsdr/hpsdr_functions.d \
./hpsdr/hpsdr_iq_tx.d \
./hpsdr/hpsdr_main.d \
./hpsdr/hpsdr_network.d \
./hpsdr/hpsdr_tx_samples.d 


# Each subdirectory must supply rules for building sources it contributes
hpsdr/%.o: ../hpsdr/%.c hpsdr/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"../hpsdr/include" -I"../librpitx-C/librpitx" -I"../librpitx-C/librpitx/core/include" -I"../librpitx-C/librpitx/modulation/include" -O3 -g3 -Wall -c -fmessage-length=0 -fcommon -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


