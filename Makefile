EXENAME          = musashi_68020_mister

MAINFILES        = musashi_68020_mister.c
MUSASHIFILES     = m68kcpu.c m68kdasm.c softfloat/softfloat.c
MUSASHIGENCFILES = m68kops.c
MUSASHIGENHFILES = m68kops.h
MUSASHIGENERATOR = m68kmake

# EXE = .exe
# EXEPATH = .\\
EXE =
EXEPATH = ./

.CFILES   = $(MAINFILES) $(OSDFILES) $(MUSASHIFILES) $(MUSASHIGENCFILES)
.OFILES   = $(.CFILES:%.c=%.o)

CC        = arm-linux-gnueabihf-gcc
WARNINGS  = -Wall -Wextra -pedantic  -g -O3 -std=c99 
#-mtune='cortex-a9' -mcpu='cortex-a9' 
CFLAGS    = $(WARNINGS)
LFLAGS    = $(WARNINGS)

TARGET = $(EXENAME)$(EXE)

DELETEFILES = $(MUSASHIGENCFILES) $(MUSASHIGENHFILES) $(.OFILES) $(TARGET) $(MUSASHIGENERATOR)$(EXE)

install: all
	scp  $(TARGET) root@192.168.0.25:/media/fat/

all: $(TARGET)

clean:
	rm -f $(DELETEFILES)

$(TARGET): $(MUSASHIGENHFILES) $(.OFILES) Makefile
	$(CC) -o $@ $(.OFILES) $(LFLAGS) -lm

$(MUSASHIGENCFILES) $(MUSASHIGENHFILES): $(MUSASHIGENERATOR)$(EXE)
	scp $(EXEPATH)$(MUSASHIGENERATOR)$(EXE)* root@192.168.0.25:/media/fat/
	scp m68k_in.c root@192.168.0.25:/media/fat/
	ssh root@192.168.0.25 /media/fat/$(MUSASHIGENERATOR)$(EXE).sh
	scp root@192.168.0.25:/media/fat/*ops* .

$(MUSASHIGENERATOR)$(EXE):  $(MUSASHIGENERATOR).c
	$(CC) -o  $(MUSASHIGENERATOR)$(EXE)  $(MUSASHIGENERATOR).c
