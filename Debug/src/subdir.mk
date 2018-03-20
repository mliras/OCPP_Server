################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/authorizationLists.c \
../src/aux.c \
../src/chargingProfile.c \
../src/data_store.c \
../src/example-server.c \
../src/ocpp_ini_parser.c \
../src/ocpp_server.c \
../src/ocpp_server_gtk.c \
../src/server_middleware.c 

OBJS += \
./src/authorizationLists.o \
./src/aux.o \
./src/chargingProfile.o \
./src/data_store.o \
./src/example-server.o \
./src/ocpp_ini_parser.o \
./src/ocpp_server.o \
./src/ocpp_server_gtk.o \
./src/server_middleware.o 

C_DEPS += \
./src/authorizationLists.d \
./src/aux.d \
./src/chargingProfile.d \
./src/data_store.d \
./src/example-server.d \
./src/ocpp_ini_parser.d \
./src/ocpp_server.d \
./src/ocpp_server_gtk.d \
./src/server_middleware.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"  -o "$@" "$<"  `pkg-config --cflags --libs gtk+-3.0`
	@echo 'Finished building: $<'
	@echo ' '


