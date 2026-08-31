
NAME   	:= buckle
SRC 	:= main.c
VERSION	:= 1.5.1
ifeq ($(OS),Windows_NT)
CC := i686-w64-mingw32-gcc
endif
PATH_AUDIO ?= "./wav"

CFLAGS	?= -O3 -g
LDFLAGS ?= -g
CFLAGS  += -Wall -Werror 
CFLAGS  += -DVERSION=\"$(VERSION)\"
CFLAGS  += -DPATH_AUDIO=\"$(PATH_AUDIO)\"

PKG_CONFIG ?= $(CROSS)pkg-config

 ifeq ($(OS),Windows_NT)
 BIN     := $(NAME).exe
 CFLAGS  += -I"win32/include"
# LDFLAGS += -mwindows -static-libgcc -static-libstdc++
 LDFLAGS += -static-libgcc -static-libstdc++
 LIBS    += -L"win32/lib" -lALURE32 -lOpenAL32
 SRC     += scan-windows.c 
else
 OS := $(shell uname)
 ifeq ($(OS), Darwin)
  BIN     := $(NAME)
  PKG_CONFIG_PATH := "./mac/lib/pkgconfig" 
  LIBS    += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) $(PKG_CONFIG) --libs alure openal)
  CFLAGS  += $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) $(PKG_CONFIG) --cflags alure openal)
  LDFLAGS += -framework ApplicationServices -framework OpenAL
  SRC     += scan-mac.c
 else
  BIN     := $(NAME)
  ifdef libinput
   LIBS    += $(shell $(PKG_CONFIG) --libs openal alure libinput libudev)
   CFLAGS  += $(shell $(PKG_CONFIG) --cflags openal alure libinput libudev)
   SRC     += scan-libinput.c
  else
   LIBS    += $(shell $(PKG_CONFIG) --libs openal alure xtst x11)
   CFLAGS  += $(shell $(PKG_CONFIG) --cflags openal alure xtst x11)
   SRC     += scan-x11.c
  endif
 endif
endif

OBJS    = $(subst .c,.o, $(SRC))
CC 	?= $(CROSS)gcc
LD 	?= $(CROSS)gcc
CCLD 	?= $(CC)
STRIP 	= $(CROSS)strip

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN):	$(OBJS)
	$(CCLD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

dist:
	mkdir -p $(NAME)-$(VERSION)
	cp -a *.c *.h wav Makefile LICENSE $(NAME)-$(VERSION)
	tar -zcf /tmp/$(NAME)-$(VERSION).tgz $(NAME)-$(VERSION)
	rm -rf $(NAME)-$(VERSION)

rec: rec.c
	$(CC) -Wall -Werror rec.c -o rec

clean:
	$(RM) $(OBJS) $(BIN) core rec

strip: $(BIN)
	$(STRIP) $(BIN)
