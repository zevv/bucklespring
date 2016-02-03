
BIN   	= buckle
SRC 	= main.c

CFLAGS  += -Wall -Werror
CFLAGS	+= -O3 -g 
LDFLAGS += -g 

LDFLAGS += $(shell pkg-config --libs openal freealut xtst x11)
CFLAGS  += $(shell pkg-config --cflags openal freealut xtst x11)

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
	
