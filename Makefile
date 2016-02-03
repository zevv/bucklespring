
BIN   	= buckle
SRC 	= main.c

CFLAGS  += -Wall -O3 -g $(shell pkg-config --cflags openal freealut x11)
CFLAGS	+= -fno-diagnostics-show-caret
LDFLAGS += -g $(shell pkg-config --libs openal freealut x11) -lXtst

CROSS	=
OBJS    = $(subst .c,.o, $(SRC))
CC 	= $(CROSS)gcc
LD 	= $(CROSS)gcc

.c.o:
	$(CC) $(CFLAGS) -c $<

$(BIN):	$(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

clean:	
	rm -f $(OBJS) $(BIN) core
	
