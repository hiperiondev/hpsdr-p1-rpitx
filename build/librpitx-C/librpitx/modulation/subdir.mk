################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../librpitx-C/librpitx/modulation/amdmasync.c \
../librpitx-C/librpitx/modulation/atv.c \
../librpitx-C/librpitx/modulation/fmdmasync.c \
../librpitx-C/librpitx/modulation/fskburst.c \
../librpitx-C/librpitx/modulation/iqdmasync.c \
../librpitx-C/librpitx/modulation/ngfmdmasync.c \
../librpitx-C/librpitx/modulation/ookburst.c \
../librpitx-C/librpitx/modulation/phasedmasync.c \
../librpitx-C/librpitx/modulation/serialdmasync.c 

OBJS += \
./librpitx-C/librpitx/modulation/amdmasync.o \
./librpitx-C/librpitx/modulation/atv.o \
./librpitx-C/librpitx/modulation/fmdmasync.o \
./librpitx-C/librpitx/modulation/fskburst.o \
./librpitx-C/librpitx/modulation/iqdmasync.o \
./librpitx-C/librpitx/modulation/ngfmdmasync.o \
./librpitx-C/librpitx/modulation/ookburst.o \
./librpitx-C/librpitx/modulation/phasedmasync.o \
./librpitx-C/librpitx/modulation/serialdmasync.o 

C_DEPS += \
./librpitx-C/librpitx/modulation/amdmasync.d \
./librpitx-C/librpitx/modulation/atv.d \
./librpitx-C/librpitx/modulation/fmdmasync.d \
./librpitx-C/librpitx/modulation/fskburst.d \
./librpitx-C/librpitx/modulation/iqdmasync.d \
./librpitx-C/librpitx/modulation/ngfmdmasync.d \
./librpitx-C/librpitx/modulation/ookburst.d \
./librpitx-C/librpitx/modulation/phasedmasync.d \
./librpitx-C/librpitx/modulation/serialdmasync.d 


# Each subdirectory must supply rules for building sources it contributes
librpitx-C/librpitx/modulation/%.o: ../librpitx-C/librpitx/modulation/%.c librpitx-C/librpitx/modulation/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Compiler'
	gcc -I"../hpsdr/include" -I"../librpitx-C/librpitx" -I"../librpitx-C/librpitx/core/include" -I"../librpitx-C/librpitx/modulation/include" -I"../mxml" -I"../filters" -O3 -g3 -Wall -c -fmessage-length=0 -fcommon -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


