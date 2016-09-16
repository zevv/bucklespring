
VERSION	:= 1.3.6
NAME   	:= buckle
SRC 	:= main.c scan-linux.c scan-windows.c scan-mac.c

PATH_AUDIO ?= "./wav"

CFLAGS  += -Wall -Werror 
CFLAGS  += -DVERSION=\"$(VERSION)\"
CFLAGS  += -DPATH_AUDIO=\"$(PATH_AUDIO)\"
CFLAGS	+= -O3 -g
LDFLAGS += -g

ifdef mingw
 BIN     := $(NAME).exe
 CROSS   := i686-w64-mingw32-
 CFLAGS  += -Iwin32/include -Iwin32/include/AL
 LDFLAGS += -mwindows -static-libgcc -static-libstdc++
 CFLAGS  += -DALURE_STATIC_LIBRARY
 LIBS    += -Lwin32/lib -lALURE32-static -lOpenAL32 
else 
 OS := $(shell uname)
 ifeq ($(OS), Darwin)
  BIN     := $(NAME)
  PKG_CONFIG_PATH := "./mac/lib/pkgconfig" 
  LDFLAGS += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs alure openal)
  CFLAGS  += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --cflags alure openal)
  LDFLAGS  += -framework ApplicationServices -framework OpenAL
 else
  BIN     := $(NAME)
  LDFLAGS += $(shell pkg-config --libs openal alure xtst x11)
  CFLAGS  += $(shell pkg-config --cflags openal alure xtst x11)
 endif
endif

OBJS    = $(subst .c,.o, $(SRC))
CC 	= $(CROSS)gcc
LD 	= $(CROSS)g++
STRIP 	= $(CROSS)strip

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN):	$(OBJS)
	$(LD) -o $@ $(OBJS) $(LIBS) $(LDFLAGS) 

dist:
	mkdir -p $(NAME)-$(VERSION)
	cp -a *.c *.h wav Makefile LICENSE $(NAME)-$(VERSION)
	tar -zcf /tmp/$(NAME)-$(VERSION).tgz $(NAME)-$(VERSION)
	rm -rf $(NAME)-$(VERSION)

rec: rec.c
	gcc -Wall -Werror rec.c -o rec

clean:
	rm -f $(OBJS) $(BIN) core rec

strip: $(BIN)
	$(STRIP) $(BIN)
