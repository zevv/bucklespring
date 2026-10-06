AM_CFLAGS = -Wall

# Every flavour looks for its samples in the same place.  This has to be built
# here rather than in configure.ac: pkgdatadir is written in terms of
# ${prefix}, which only make expands.
buckle_cppflags = -DPATH_AUDIO='"$(wavdir)"'

bin_PROGRAMS =
dist_man1_MANS =
dist_man8_MANS =
CLEANFILES =

# One program per source of key events.  main.c is shared, so per target flags
# are what keeps them apart; automake gives each its own object files.
# The tray is the same for every flavour, so it joins each of them rather than
# being a flavour of its own.
if HAVE_TRAY
tray_cflags = $(TRAY_CFLAGS) $(PTHREAD_CFLAGS)
tray_libs = $(TRAY_LIBS)
else
tray_cflags =
tray_libs =
endif

buckle_x11_SOURCES = main.c buckle.h scan-x11.c
buckle_x11_CPPFLAGS = $(buckle_cppflags)
buckle_x11_CFLAGS = $(AM_CFLAGS) $(AUDIO_CFLAGS) $(X11_CFLAGS) $(tray_cflags)
buckle_x11_LDADD = $(AUDIO_LIBS) $(X11_LIBS) $(tray_libs)

buckle_libinput_SOURCES = main.c buckle.h scan-libinput.c wl-repeat.c
buckle_libinput_CPPFLAGS = $(buckle_cppflags)
buckle_libinput_CFLAGS = $(AM_CFLAGS) $(AUDIO_CFLAGS) $(LIBINPUT_CFLAGS) $(tray_cflags)
buckle_libinput_LDADD = $(AUDIO_LIBS) $(LIBINPUT_LIBS) $(tray_libs)

buckle_mac_SOURCES = main.c buckle.h scan-mac.c
buckle_mac_CPPFLAGS = $(buckle_cppflags)
buckle_mac_CFLAGS = $(AM_CFLAGS) $(AUDIO_CFLAGS) $(tray_cflags)
buckle_mac_LDADD = $(AUDIO_LIBS) $(MAC_LIBS) $(tray_libs)

buckle_windows_SOURCES = main.c buckle.h scan-windows.c
buckle_windows_CPPFLAGS = $(buckle_cppflags)
buckle_windows_CFLAGS = $(AM_CFLAGS) $(AUDIO_CFLAGS) $(tray_cflags)
buckle_windows_LDFLAGS = $(WINDOWS_LDFLAGS)
buckle_windows_LDADD = $(AUDIO_LIBS) $(tray_libs)

if HAVE_TRAY
buckle_x11_SOURCES += tray.c
buckle_libinput_SOURCES += tray.c
buckle_mac_SOURCES += tray.c
buckle_windows_SOURCES += tray.c
endif

desktopdir = $(datadir)/applications
dist_desktop_DATA =

if BACKEND_X11
bin_PROGRAMS += buckle-x11
dist_man1_MANS += man/buckle-x11.1
dist_desktop_DATA += buckle-x11.desktop
endif

if BACKEND_LIBINPUT
bin_PROGRAMS += buckle-libinput
dist_man1_MANS += man/buckle-libinput.1
dist_desktop_DATA += buckle-libinput.desktop
endif

if BACKEND_MAC
bin_PROGRAMS += buckle-mac
endif

if BACKEND_WINDOWS
bin_PROGRAMS += buckle-windows
endif

# The systemd user units, and the manual pages that document them, follow the
# programs they start.  The units are generated so that ExecStart names the
# bindir this was configured for; the [@] keeps config.status from eating the
# token on its way through Makefile.in.
subst = $(SED) \
	-e 's|@bindir[@]|$(bindir)|g' \
	-e 's|@sysconfdir[@]|$(sysconfdir)|g'

if INSTALL_SYSTEMD
systemduserunit_DATA =
if BACKEND_X11
systemduserunit_DATA += systemd/buckle-x11.service
dist_man8_MANS += man/buckle-x11.service.8
endif
if BACKEND_LIBINPUT
systemduserunit_DATA += systemd/buckle-libinput.service
dist_man8_MANS += man/buckle-libinput.service.8
endif
CLEANFILES += $(systemduserunit_DATA)
endif

systemd/buckle-x11.service: $(srcdir)/systemd/buckle-x11.service.in Makefile
	$(AM_V_GEN)$(MKDIR_P) systemd && $(subst) < $< > $@

systemd/buckle-libinput.service: $(srcdir)/systemd/buckle-libinput.service.in Makefile
	$(AM_V_GEN)$(MKDIR_P) systemd && $(subst) < $< > $@

# The uaccess rule is only of use to the flavour that reads the raw devices.
# It does nothing until the administrator creates the flag file; read the
# comments in it before arming it.
if INSTALL_UDEV
if BACKEND_LIBINPUT
udevrules_DATA = udev/70-bucklespring-libinput.rules
CLEANFILES += $(udevrules_DATA)
endif
endif

udev/70-bucklespring-libinput.rules: $(srcdir)/udev/70-bucklespring-libinput.rules.in Makefile
	$(AM_V_GEN)$(MKDIR_P) udev && $(subst) < $< > $@

# The manual pages for the executables are generated from their own --help and
# --version output, so an option added to the program turns up in its page
# without anyone writing it down twice.  help2man has to run the program, which
# a cross build cannot do, so the pages are kept in the tree and refreshed on
# demand.  A page that changes here is telling you to commit it.
H2M = $(HELP2MAN) --no-info \
	--source="Bucklespring keyboard sound" --manual="User Commands"

.PHONY: update-man
update-man: $(bin_PROGRAMS)
	@test "x$(HELP2MAN)" != xno || \
	  { echo "update-man needs help2man; none was found by configure" >&2; \
	    exit 1; }
if BACKEND_X11
	$(H2M) --include=$(srcdir)/man/buckle-x11.1.h2m \
		--output=$(srcdir)/man/buckle-x11.1 ./buckle-x11$(EXEEXT)
endif
if BACKEND_LIBINPUT
	$(H2M) --include=$(srcdir)/man/buckle-libinput.1.h2m \
		--output=$(srcdir)/man/buckle-libinput.1 ./buckle-libinput$(EXEEXT)
endif

# The sample recorder, built only when asked for: make rec
EXTRA_PROGRAMS = rec
rec_SOURCES = rec.c


EXTRA_DIST = README.md LICENSE \
	img/buckle.gif img/icon.png img/icon128.ico img/model-m.jpg \
	mac/lib/pkgconfig/openal.pc \
	man/buckle-x11.1.h2m man/buckle-libinput.1.h2m \
	systemd/buckle-x11.service.in systemd/buckle-libinput.service.in \
	udev/70-bucklespring-libinput.rules.in

wavdir = $(pkgdatadir)/wav
dist_wav_DATA = wav/01-0.wav wav/01-1.wav wav/02-0.wav wav/02-1.wav	\
	wav/03-0.wav wav/03-1.wav wav/04-0.wav wav/04-1.wav		\
	wav/05-0.wav wav/05-1.wav wav/06-0.wav wav/06-1.wav		\
	wav/07-0.wav wav/07-1.wav wav/08-0.wav wav/08-1.wav		\
	wav/09-0.wav wav/09-1.wav wav/0a-0.wav wav/0a-1.wav		\
	wav/0b-0.wav wav/0b-1.wav wav/0c-0.wav wav/0c-1.wav		\
	wav/0d-0.wav wav/0d-1.wav wav/0e-0.wav wav/0e-1.wav		\
	wav/0f-0.wav wav/0f-1.wav wav/10-0.wav wav/10-1.wav		\
	wav/11-0.wav wav/11-1.wav wav/12-0.wav wav/12-1.wav		\
	wav/13-0.wav wav/13-1.wav wav/14-0.wav wav/14-1.wav		\
	wav/15-0.wav wav/15-1.wav wav/16-0.wav wav/16-1.wav		\
	wav/17-0.wav wav/17-1.wav wav/18-0.wav wav/18-1.wav		\
	wav/19-0.wav wav/19-1.wav wav/1a-0.wav wav/1a-1.wav		\
	wav/1b-0.wav wav/1b-1.wav wav/1c-0.wav wav/1c-1.wav		\
	wav/1d-0.wav wav/1d-1.wav wav/1e-0.wav wav/1e-1.wav		\
	wav/1f-0.wav wav/1f-1.wav wav/20-0.wav wav/20-1.wav		\
	wav/21-0.wav wav/21-1.wav wav/22-0.wav wav/22-1.wav		\
	wav/23-0.wav wav/23-1.wav wav/24-0.wav wav/24-1.wav		\
	wav/25-0.wav wav/25-1.wav wav/26-0.wav wav/26-1.wav		\
	wav/27-0.wav wav/27-1.wav wav/28-0.wav wav/28-1.wav		\
	wav/29-0.wav wav/29-1.wav wav/2a-0.wav wav/2a-1.wav		\
	wav/2b-0.wav wav/2b-1.wav wav/2c-0.wav wav/2c-1.wav		\
	wav/2d-0.wav wav/2d-1.wav wav/2e-0.wav wav/2e-1.wav		\
	wav/2f-0.wav wav/2f-1.wav wav/30-0.wav wav/30-1.wav		\
	wav/31-0.wav wav/31-1.wav wav/32-0.wav wav/32-1.wav		\
	wav/33-0.wav wav/33-1.wav wav/34-0.wav wav/34-1.wav		\
	wav/35-0.wav wav/35-1.wav wav/36-0.wav wav/36-1.wav		\
	wav/37-0.wav wav/37-1.wav wav/38-0.wav wav/38-1.wav		\
	wav/39-0.wav wav/39-1.wav wav/3a-0.wav wav/3a-1.wav		\
	wav/3b-0.wav wav/3b-1.wav wav/3c-0.wav wav/3c-1.wav		\
	wav/3d-0.wav wav/3d-1.wav wav/3e-0.wav wav/3e-1.wav		\
	wav/3f-0.wav wav/3f-1.wav wav/40-0.wav wav/40-1.wav		\
	wav/41-0.wav wav/41-1.wav wav/42-0.wav wav/42-1.wav		\
	wav/43-0.wav wav/43-1.wav wav/44-0.wav wav/44-1.wav		\
	wav/45-0.wav wav/45-1.wav wav/46-0.wav wav/46-1.wav		\
	wav/47-0.wav wav/47-1.wav wav/48-0.wav wav/48-1.wav		\
	wav/49-0.wav wav/49-1.wav wav/4a-0.wav wav/4a-1.wav		\
	wav/4b-0.wav wav/4b-1.wav wav/4c-0.wav wav/4c-1.wav		\
	wav/4d-0.wav wav/4d-1.wav wav/4e-0.wav wav/4e-1.wav		\
	wav/4f-0.wav wav/4f-1.wav wav/50-0.wav wav/50-1.wav		\
	wav/51-0.wav wav/51-1.wav wav/52-0.wav wav/52-1.wav		\
	wav/53-0.wav wav/53-1.wav wav/56-0.wav wav/56-1.wav		\
	wav/57-0.wav wav/57-1.wav wav/58-0.wav wav/58-1.wav		\
	wav/5b-0.wav wav/5b-1.wav wav/60-0.wav wav/60-1.wav		\
	wav/61-0.wav wav/61-1.wav wav/62-0.wav wav/62-1.wav		\
	wav/63-0.wav wav/63-1.wav wav/64-0.wav wav/64-1.wav		\
	wav/66-0.wav wav/66-1.wav wav/67-0.wav wav/67-1.wav		\
	wav/68-0.wav wav/68-1.wav wav/69-0.wav wav/69-1.wav		\
	wav/6a-0.wav wav/6a-1.wav wav/6b-0.wav wav/6b-1.wav		\
	wav/6c-0.wav wav/6c-1.wav wav/6d-0.wav wav/6d-1.wav		\
	wav/6e-0.wav wav/6e-1.wav wav/6f-0.wav wav/6f-1.wav		\
	wav/77-0.wav wav/7d-0.wav wav/7d-1.wav wav/ff-0.wav		\
	wav/ff-1.wav
