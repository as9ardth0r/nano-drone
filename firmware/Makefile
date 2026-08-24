TARGET = nanodrone_fc
BUILD_DIR = build

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

MCU_FLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

C_SOURCES = \
	Core/Src/main.c \
	Core/Src/clock.c \
	Core/Src/gpio.c \
	Core/Src/i2c_bus.c \
	Core/Src/imu_mpu6050.c \
	Core/Src/tof_array.c \
	Core/Src/motors_pwm.c \
	Core/Src/avoidance.c \
	Core/Src/command_link.c \
	Core/Src/uart3.c \
	Core/Src/system_stm32f4xx.c

ASM_SOURCES = startup/startup_stm32f405xx.s

C_INCLUDES = \
	-ICore/Inc \
	-IDrivers \
	-IDrivers/CMSIS_Core

C_DEFS = -DSTM32F405xx

CFLAGS = $(MCU_FLAGS) $(C_DEFS) $(C_INCLUDES) -Wall -Wextra -O2 -std=c11 -ffunction-sections -fdata-sections -g

LDSCRIPT = startup/STM32F405RGTx_FLASH.ld
LDFLAGS = $(MCU_FLAGS) -specs=nano.specs -T$(LDSCRIPT) -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map -lm

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(CC) -c $(MCU_FLAGS) -x assembler-with-cpp $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
