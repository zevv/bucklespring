
NAME   	= buckle
SRC 	= main.c scan-linux.c scan-windows.c

CFLAGS  += -Wall -Werror
CFLAGS	+= -O3 -g
LDFLAGS += -g

ifdef mingw
 BIN     := $(NAME).exe
 CROSS   := i686-w64-mingw32-
 CFLAGS  += -Iwin32/include
 CFLAGS  += -Ifreealut-1.1.0/include
 CFLAGS  += -DHAVE_WINDOWS_H -DHAVE_SLEEP -DHAVE_STAT
 LIBS    += -Lwin32/lib -lalut -lOpenAL32 
else
 BIN     := $(NAME)
 LDFLAGS += $(shell pkg-config --libs openal freealut xtst x11)
 CFLAGS  += $(shell pkg-config --cflags openal freealut xtst x11)
endif

OBJS    = $(subst .c,.o, $(SRC))
CC 	= $(CROSS)gcc
LD 	= $(CROSS)gcc
STRIP 	= $(CROSS)strip

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN):	$(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

rec: rec.c
	gcc -Wall -Werror rec.c -o rec

clean:
	rm -f $(OBJS) $(BIN) core rec

strip: $(BIN)
	$(STRIP) $(BIN)
