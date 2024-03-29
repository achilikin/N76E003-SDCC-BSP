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

## pin to set time markers
MARK_PIN  = P04

BSPROOT = ../..
BSPDIR  = $(BSPROOT)/bsp
LIBDIR  = $(BSPROOT)/lib

SRCS  = $(BSPDIR)/N76E003.c
SRCS += $(BSPDIR)/tick.c
SRCS += $(BSPDIR)/uart.c
SRCS += $(BSPDIR)/event.c
SRCS += $(BSPDIR)/terminal.c

SRCS += $(wildcard *.c)

OBJS = $(SRCS:.c=.rel)

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
ifneq ($(HIRC_TRIM),false)
CFLAGS += -DHIRC_TRIM=$(HIRC_TRIM)
endif

ifneq ($(MARK_PIN),false)
CFLAGS += -DMARK_PIN=$(MARK_PIN)
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
