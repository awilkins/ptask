#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT([ptask], [1.0.0],[jeanfi@gmail.com],[ptask],[http://wpitchoune.net/ptask])

AM_INIT_AUTOMAKE([-Wall gnu])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O

# Checks for header files.
AC_PATH_X

AM_GNU_GETTEXT_VERSION([0.16])
AM_GNU_GETTEXT([external])

############### common 

PKG_CHECK_MODULES(X11, x11)
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

PKG_CHECK_MODULES(XEXT, xext)
AC_SUBST(XEXT_CFLAGS)
AC_SUBST(XEXT_LIBS)

# Checks GTK+ 3.0
PKG_CHECK_MODULES(GTK, [gtk+-3.0 >= 3.12])
# ensure that only allowed headers are included
GTK_CFLAGS+=" -DGTK_DISABLE_SINGLE_INCLUDES "
# ensure that no gtk deprecated symbols are used
GTK_CFLAGS+=" -DGDK_DISABLE_DEPRECATED "
# GTK_CFLAGS+=" -DGTK_DISABLE_DEPRECATED "
# ensure use of accessors
GTK_CFLAGS+=" -DGSEAL_ENABLE "
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

# Check JSON
JSON_LIBS=
PKG_CHECK_MODULES(JSON, 
		  json-c, 
		  [AC_DEFINE([HAVE_JSON],[1],[Use JSON])], 
		  [AC_MSG_WARN("json-c missing")])

if test "$JSON_LIBS" == ""; then
PKG_CHECK_MODULES(JSON, 
		  json, 
		  [AC_DEFINE([HAVE_JSON],[1],[Use JSON])
		   AC_DEFINE([HAVE_JSON_0],[1],[Use JSON0])], 
		  [AC_MSG_WARN("json0 missing")])
fi

AM_CONDITIONAL(JSON, test -n "$JSON_LIBS")
AC_SUBST(JSON_CFLAGS)
AC_SUBST(JSON_LIBS)

# Gsettings
GLIB_GSETTINGS

AC_CONFIG_FILES([
 Makefile
 icons/hicolor/scalable/Makefile
 po/Makefile.in
 src/Makefile
 src/glade/Makefile
 tests/Makefile
])

AC_CHECK_PROGS([HELP2MAN], [help2man])

AC_CHECK_PROG(HAVE_CPPCHECK, cppcheck, yes)
AM_CONDITIONAL(CPPCHECK, test -n "$HAVE_CPPCHECK")

AC_OUTPUT
