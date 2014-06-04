CC	= msp430-gcc
OBJCOPY	= msp430-objcopy
OBJDUMP	= msp430-objdump
SIZE	= msp430-size --target=elf32-msp430
MCU	= msp430g2452

DEBUG=1

MSPPROG = mspdebug rf2500

# Level of Optimization
OPT = s

CFLAGS += -mmcu=$(MCU) -O$(OPT)
CFLAGS += -Wextra
CFLAGS += -Wall -Wcast-align -Wimplicit
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wpointer-arith -Wswitch
CFLAGS += -Wredundant-decls -Wreturn-type
CFLAGS += -Wshadow -Wunused
CFLAGS += -fno-builtin
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -fpack-struct

PROG=main
OBJS=$(PROG).o usi_i2c.o msp_iface.o
SRCS=$(PROG).c usi_i2c.c msp_iface.c
ifeq ($(DEBUG), 1)
CFLAGS += -D DEBUG
OBJS=serial.o $(PROG).o printf.o usi_i2c.o msp_iface.o
SRCS=serial.s $(PROG).c printf.c usi_i2c.c msp_iface.c
endif


CLEANFILES=$(PROG).elf $(OBJS) *.o *.lss *.elf

$(PROG).elf: $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG).elf $(OBJS)
	@echo "___ [SIZE] ______________________________________________"
	$(SIZE) $@
	@echo "___ [SIZE] ______________________________________________"

# need latest uniarch patches for the -x arg to work
serial.o: serial.s
	$(CC) -c -x assembler-with-cpp -Wa,-al=serial.lss -o serial.o serial.s

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

install: $(PROG).elf
	$(MSPPROG) 'prog $(PROG).elf'


clean:
	rm -f $(CLEANFILES)
