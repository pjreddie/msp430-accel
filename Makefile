CC	= msp430-gcc
OBJCOPY	= msp430-objcopy
OBJDUMP	= msp430-objdump
SIZE	= msp430-size --target=elf32-msp430
MCU	= msp430g2452

MSPPROG = mspdebug rf2500

# Level of Optimization
OPT = s

CFLAGS += -mmcu=$(MCU) -O$(OPT)
CFLAGS += -Werror -Wextra
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
OBJS=$(PROG).o printf.o
SRCS=$(PROG).c printf.c


CLEANFILES=$(PROG).elf $(OBJS) *.o *.lss *.elf

$(PROG).elf: $(OBJS) serial.o
	$(CC) $(CFLAGS) -o $(PROG).elf $(OBJS) serial.o
	@echo "___ [SIZE] ______________________________________________"
	$(SIZE) $@
	@echo "___ [SIZE] ______________________________________________"

$(OBJS): $(SRCS) 
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(SRCS)

install: $(PROG).elf
	$(MSPPROG) 'prog $(PROG).elf'

# need latest uniarch patches for the -x arg to work
serial.o: serial.s
	$(CC) -c -x assembler-with-cpp -Wa,-al=serial.lss -o serial.o serial.s

clean:
	rm -f $(CLEANFILES)
