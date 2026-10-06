AC_INIT([bucklespring],[1.5.2~dev],[https://github.com/barak/bucklespring/issues],
        [bucklespring],[https://github.com/barak/bucklespring])

AC_CONFIG_SRCDIR([main.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign -Wall])

AC_CANONICAL_HOST
AC_PROG_CC
AC_PROG_SED
PKG_PROG_PKG_CONFIG

AS_CASE([$host_os],
  [mingw*|cygwin*|msys*], [host_kind=windows],
  [darwin*],              [host_kind=mac],
  [linux*],               [host_kind=linux],
                          [host_kind=other])

dnl One executable per source of key events: they differ only in where the
dnl events come from, and a machine can want more than one of them, an X
dnl session and a Wayland session on the same box reaching for a different
dnl one.  So each is a separate program rather than a configure time choice
dnl of what the single binary contains, and each defaults to being built
dnl wherever it could work and its dependencies are installed.

AC_ARG_ENABLE([x11],
  [AS_HELP_STRING([--enable-x11],[build buckle-x11 @<:@auto@:>@])],
  [], [enable_x11=auto])
AC_ARG_ENABLE([libinput],
  [AS_HELP_STRING([--enable-libinput],[build buckle-libinput @<:@auto@:>@])],
  [], [enable_libinput=auto])
AC_ARG_ENABLE([mac],
  [AS_HELP_STRING([--enable-mac],[build buckle-mac @<:@auto@:>@])],
  [], [enable_mac=auto])
AC_ARG_ENABLE([windows],
  [AS_HELP_STRING([--enable-windows],[build buckle-windows @<:@auto@:>@])],
  [], [enable_windows=auto])

dnl mac and windows are decided by the host alone; there is nothing to probe.
AS_IF([test "x$enable_mac" = xauto],
  [AS_IF([test "x$host_kind" = xmac], [enable_mac=yes], [enable_mac=no])])
AS_IF([test "x$enable_windows" = xauto],
  [AS_IF([test "x$host_kind" = xwindows], [enable_windows=yes], [enable_windows=no])])

dnl Every backend plays its sounds the same way.  Only the Windows build gets
dnl its OpenAL out of the tree rather than from pkg-config, and win32/ is not
dnl shipped here, so that one is wired up but untested.
AS_IF([test "x$enable_windows" = xyes],
  [AUDIO_CFLAGS='-I$(top_srcdir)/win32/include -I$(top_srcdir)/win32/include/AL'
   AUDIO_LIBS='-L$(top_srcdir)/win32/lib -lALURE32 -lOpenAL32'
   AC_SUBST([AUDIO_CFLAGS])
   AC_SUBST([AUDIO_LIBS])],
  [PKG_CHECK_MODULES([AUDIO],[openal alure])])

AS_IF([test "x$enable_x11" = xauto],
  [AS_CASE([$host_kind], [linux|other], [], [enable_x11=no])])
AS_IF([test "x$enable_x11" != xno],
  [PKG_CHECK_MODULES([X11],[x11 xtst],
     [enable_x11=yes],
     [AS_IF([test "x$enable_x11" = xyes],
            [AC_MSG_ERROR([--enable-x11 given but: $X11_PKG_ERRORS])],
            [enable_x11=no])])])

AS_IF([test "x$enable_libinput" = xauto && test "x$host_kind" != xlinux],
  [enable_libinput=no])
AS_IF([test "x$enable_libinput" != xno],
  [PKG_CHECK_MODULES([LIBINPUT],[libinput libudev wayland-client],
     [enable_libinput=yes],
     [AS_IF([test "x$enable_libinput" = xyes],
            [AC_MSG_ERROR([--enable-libinput given but: $LIBINPUT_PKG_ERRORS])],
            [enable_libinput=no])])])

AS_IF([test "x$enable_mac" = xyes],
  [MAC_LIBS="-framework ApplicationServices -framework OpenAL"])
AC_SUBST([MAC_LIBS])
AS_IF([test "x$enable_windows" = xyes],
  [WINDOWS_LDFLAGS="-mwindows -static-libgcc -static-libstdc++"])
AC_SUBST([WINDOWS_LDFLAGS])

dnl An icon in the system tray, with a menu for pausing, the volume and the
dnl output device.  It needs GTK and libayatana-appindicator, which is a good
dnl deal more than the rest of the program does, so it is built when they are
dnl there and quietly left out when they are not.
AC_ARG_ENABLE([tray],
  [AS_HELP_STRING([--enable-tray],[build the system tray icon @<:@auto@:>@])],
  [], [enable_tray=auto])
AS_IF([test "x$enable_tray" != xno],
  [PKG_CHECK_MODULES([TRAY],[gtk+-3.0 ayatana-appindicator3-0.1],
     [enable_tray=yes
      AC_DEFINE([HAVE_TRAY],[1],[Define to build the system tray icon.])],
     [AS_IF([test "x$enable_tray" = xyes],
            [AC_MSG_ERROR([--enable-tray given but: $TRAY_PKG_ERRORS])],
            [enable_tray=no])])])
AM_CONDITIONAL([HAVE_TRAY],[test "x$enable_tray" = xyes])

dnl The tray runs the GTK loop on the main thread and the key event loop on
dnl one of its own, so the program needs threads once the tray is in it.
dnl -pthread covers every compiler that can reach GTK in the first place,
dnl which is why this is not AX_PTHREAD and no autoconf-archive is needed.
AS_IF([test "x$enable_tray" = xyes], [
  buckle_save_CFLAGS=$CFLAGS
  CFLAGS="$CFLAGS -pthread"
  AC_MSG_CHECKING([whether $CC accepts -pthread])
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[#include <pthread.h>]],
       [[pthread_t t; pthread_create(&t, 0, 0, 0);]])],
    [AC_MSG_RESULT([yes])
     PTHREAD_CFLAGS=-pthread],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([the system tray needs POSIX threads])])
  CFLAGS=$buckle_save_CFLAGS
])
AC_SUBST([PTHREAD_CFLAGS])

AS_IF([test "x$enable_x11" = xno && test "x$enable_libinput" = xno &&
       test "x$enable_mac" = xno && test "x$enable_windows" = xno],
  [AC_MSG_ERROR([no backend enabled, so there would be nothing to build])])

AM_CONDITIONAL([BACKEND_X11],      [test "x$enable_x11" = xyes])
AM_CONDITIONAL([BACKEND_LIBINPUT], [test "x$enable_libinput" = xyes])
AM_CONDITIONAL([BACKEND_MAC],      [test "x$enable_mac" = xyes])
AM_CONDITIONAL([BACKEND_WINDOWS],  [test "x$enable_windows" = xyes])

dnl Only needed by "make update-man", never by a plain build: the pages are
dnl kept in the tree because help2man has to run the program it documents,
dnl which a cross build cannot do.
AC_CHECK_PROGS([HELP2MAN],[help2man],[no])

dnl Both of these default under ${prefix} rather than to what pkg-config
dnl reports, so that a --prefix=$HOME build does not try to write into
dnl /usr/lib.  Packagers will want to pass the system directory.  systemd
dnl reads user units from $prefix/lib/systemd/user for /usr and /usr/local
dnl alike.  Pass no to skip either.
AC_ARG_WITH([systemduserunitdir],
  [AS_HELP_STRING([--with-systemduserunitdir=DIR],
     [systemd user units, or no @<:@PREFIX/lib/systemd/user@:>@])],
  [], [with_systemduserunitdir='${prefix}/lib/systemd/user'])
AC_ARG_WITH([udevrulesdir],
  [AS_HELP_STRING([--with-udevrulesdir=DIR],
     [udev rules, or no @<:@PREFIX/lib/udev/rules.d@:>@])],
  [], [with_udevrulesdir='${prefix}/lib/udev/rules.d'])

systemduserunitdir=$with_systemduserunitdir
udevrulesdir=$with_udevrulesdir
AC_SUBST([systemduserunitdir])
AC_SUBST([udevrulesdir])
AM_CONDITIONAL([INSTALL_SYSTEMD], [test "x$with_systemduserunitdir" != xno])
AM_CONDITIONAL([INSTALL_UDEV],    [test "x$with_udevrulesdir" != xno])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_MSG_NOTICE([
  buckle-x11 ........ $enable_x11
  buckle-libinput ... $enable_libinput
  buckle-mac ........ $enable_mac
  buckle-windows .... $enable_windows
  system tray ....... $enable_tray
  help2man .......... $HELP2MAN (only for "make update-man")
  systemd units ..... $with_systemduserunitdir
  udev rules ........ $with_udevrulesdir
])
