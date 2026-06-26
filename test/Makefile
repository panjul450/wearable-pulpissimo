# ============================================================================
# ICDeC Gyroscope Sensor Library - Test Makefile
#
# Build and run test applications for L3G4200D and MPU-6050 sensors
# on the ICDeC PULPissimo FPGA board.
#
# Usage:
#   make all SENSOR=l3g4200d              Build L3G4200D test
#   make all SENSOR=mpu6050               Build MPU-6050 test
#   make run SENSOR=l3g4200d platform=fpga   Run L3G4200D test on FPGA
#   make run SENSOR=mpu6050 platform=fpga    Run MPU-6050 test on FPGA
#   make clean SENSOR=                           Clean build artifacts
# ============================================================================

# Validate SENSOR selection
ifndef SENSOR
$(error SENSOR is not set. Use SENSOR=l3g4200d or SENSOR=mpu6050)
endif

ifeq ($(SENSOR),l3g4200d)
PULP_APP = test_l3g4200d
PULP_APP_C_SRCS = test_l3g4200d.c ../src/l3g4200d.c
else ifeq ($(SENSOR),mpu6050)
PULP_APP = test_mpu6050
PULP_APP_C_SRCS = test_mpu6050.c ../src/mpu6050.c
else
$(error Invalid SENSOR value '$(SENSOR)'. Use SENSOR=l3g4200d or SENSOR=mpu6050)
endif

# Include Arduino time functions (provides micros() needed by I2C driver)
PULP_APP_C_SRCS += $(PULPRT_HOME)/arduino/cores/arduino_time.c

# Compiler flags
PULP_CFLAGS = -O3 -g -I../include -I$(PULPRT_HOME)/arduino/cores

# Include pulp-runtime build rules
include $(PULPRT_HOME)/rules/pulp.mk
