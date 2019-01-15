CC       = arm-eabi-gcc
OBJCOPY  = arm-eabi-objcopy
FLAGS    = -mthumb -mcpu=cortex-m4
CPPFLAGS = -DSTM32F411xE
CFLAGS   = $(FLAGS) -Wall -g \
		   -O2 -ffunction-sections -fdata-sections \
		   -I/opt/arm/stm32/inc \
		   -I/opt/arm/stm32/CMSIS/Include \
		   -I/opt/arm/stm32/CMSIS/Device/ST/STM32F4xx/Include
LDFLAGS  = $(FLAGS) -Wl,--gc-sections -nostartfiles \
		   -L/opt/arm/stm32/lds -Tstm32f411re.lds
vpath %.c /opt/arm/stm32/src

OBJECTS = prog.o startup_stm32.o gpio.o leds.o buttons.o usart.o queue.o timers.o
TARGET  = prog

.SECONDARY: $(TARGET).elf $(OBJECTS)

all: $(TARGET).bin

%.elf : $(OBJECTS)
		$(CC) $(LDFLAGS) $^ -o $@

%.bin : %.elf
		$(OBJCOPY) $< $@ -O binary

clean :
		rm -f *.bin *.elf *.hex *.d *.o *.bak *~
