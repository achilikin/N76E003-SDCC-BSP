## some configuration defines
IMAGE     = main
MCU       = N76E003AT20
ARCH      = mcs51
## set system clock to 16.0 or 16.6 MHz
## F_OSC  = FOSC_16000
F_OSC     = FOSC_16600

## devboard uses UART 0
USE_UART  = 0
## handpicked RC trim value
HIRC_TRIM = 19

## use BV4618 LCD controller board
USE_BV4618_LCD = true
## use PCF8574 LCD controller board
USE_PCF8574_LCD = true
PCF8574_LINES = 4
PCF8574_CHARS = 20
## set to true to compile debug calls
MEM_DEBUG = true
DHT_DEBUG = false
DHT_PIN  = P05
## set TICK_DEBUG to a pin name to enable 1ms output
## or to false to disable it
TICK_DEBUG = P12

BSPROOT = ../..
BSPDIR  = $(BSPROOT)/bsp
LIBDIR  = $(BSPROOT)/lib

SRCS  = $(BSPDIR)/N76E003.c
SRCS += $(BSPDIR)/adc.c
SRCS += $(BSPDIR)/i2c.c
SRCS += $(BSPDIR)/tick.c
SRCS += $(BSPDIR)/uart.c
SRCS += $(BSPDIR)/event.c
SRCS += $(BSPDIR)/terminal.c
SRCS += $(BSPDIR)/iap_read.c
SRCS += $(BSPDIR)/iap_write.c
SRCS += $(BSPDIR)/pinterrupt.c

SRCS += $(LIBDIR)/dump.c
SRCS += $(LIBDIR)/sfrs.c
SRCS += $(LIBDIR)/dht.c
SRCS += $(LIBDIR)/ds3231.c
SRCS += $(LIBDIR)/i2c_mem.c
ifeq ($(USE_BV4618_LCD),true)
SRCS += $(LIBDIR)/bv4618.c
endif
ifeq ($(USE_PCF8574_LCD),true)
SRCS += $(LIBDIR)/pcf8574.c
endif

SRCS += $(wildcard *.c)

OBJS = $(SRCS:.c=.rel)

#ASRCS := $(wildcard *.s $(LIBDIR)/*.s)
#OBJS += $(ASRCS:.s=.rel)

CC	= sdcc
LD	= sdld
AS	= sdas8051
ICP = nulink.exe

## uncomment '--stack-auto' to place functions parameters
## on the stack automatically
## if commented out then use '__reentrant' per function declaration
## to use stack for function's parameters
# STACK = --stack-auto
## also stack position can be specified explicitly:
# STACK	= --stack-loc 0x80 --no-pack-iram

## MS Windows style
RM       = del
FORCE    = /F /Q
## Linux style
#RM       = rm
#FORCE    = -f

SIZE     = python $(BSPROOT)/pys/size-$(ARCH).py
OBJCOPY  = sdobjcopy
ASFLAGS  = -plosgff
CFLAGS  += -m$(ARCH) -p$(MCU) --std-sdcc11
CFLAGS  += -D$(F_OSC) -DDHT_PIN=$(DHT_PIN)
CFLAGS  += -I. -I$(BSPDIR) -I$(LIBDIR)
CFLAGS  += $(STACK)
CFLAGS  += --fomit-frame-pointer
## looks like --opt-code-xxx does not do anything
## but use --opt-code-size just in case of support in the future
CFLAGS += --opt-code-size
#CFLAGS += --opt-code-speed
CFLAGS += -DUSE_UART=$(USE_UART)

## include some debug interfaces if enabled
ifeq ($(MEM_DEBUG),true)
CFLAGS += -DMEM_DEBUG
endif
ifeq ($(DHT_DEBUG),true)
CFLAGS += -DDHT_DEBUG
endif
ifneq ($(TICK_DEBUG),false)
CFLAGS += -DTICK_DEBUG=$(TICK_DEBUG)
endif
ifneq ($(HIRC_TRIM),false)
CFLAGS += -DHIRC_TRIM=$(HIRC_TRIM)
endif
ifeq ($(USE_BV4618_LCD),true)
CFLAGS += -DUSE_BV4618_LCD
endif
ifeq ($(USE_PCF8574_LCD),true)
CFLAGS += -DUSE_PCF8574_LCD -DPCF8574_LINES=$(PCF8574_LINES) -DPCF8574_CHARS=$(PCF8574_CHARS)
endif

LDFLAGS  = -m$(ARCH) -l$(ARCH) --out-fmt-ihx
LDFLAGS  += --iram-size 256 --xram-size 768 --code-size 18432
LDFLAGS  += $(STACK)

all: $(IMAGE).ihx

$(IMAGE).ihx: $(OBJS) Makefile
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	@packihx $@ > $(IMAGE).hex
	@makebin -p $(IMAGE).hex $(IMAGE).bin
	@$(SIZE) $(IMAGE).mem

%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.rel: %.s
	$(AS) $(ASFLAGS) $<

-include $(IMAGE).dep

size:
	@$(SIZE) $(IMAGE).mem

clean:
	$(RM) $(FORCE) *.asm *.lst *.rel *.rst *.sym *.hex *.bin *.ihx *.lk *.map *.mem
	$(MAKE) -C $(BSPDIR)/ clean
	$(MAKE) -C $(LIBDIR)/ clean

## reset device
reset:
	$(ICP) -reset

## erase everything on the chip
## usually needed only once for the fresh device
erase:
	$(ICP) -e all

## flash new image to the device
install: $(IMAGE).ihx
	$(ICP) -e APROM
	$(ICP) -w APROM $(IMAGE).bin

.PHONY: all size clean reset erase install
