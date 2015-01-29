m4_define([ghbci_major_version], [0])
m4_define([ghbci_minor_version], [0])
m4_define([ghbci_micro_version], [1])

m4_define([ghbci_version], [ghbci_major_version.ghbci_minor_version.ghbci_micro_version])

AC_PREREQ(2.69)

AC_INIT([GHbci], [ghbci_version], [florian@richter-es.de])

AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build/autotools])
AC_CONFIG_SRCDIR([ghbci])
AC_CONFIG_HEADER([config.h])

# Checks for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

# Checks for libraries
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.38])

# Checks for typedefs, structures, and compiler characteristics
AC_PROG_CC_STDC

# JNI paths (TODO: should be autodetected)
JAVA_HOME="/usr/lib/jvm/java"
JAVA_LIBS="-R$JAVA_HOME/jre/lib/amd64/server -L$JAVA_HOME/jre/lib/amd64/server -ljvm"
#JAVA_LIBS="-L$JAVA_HOME/jre/lib/amd64/server -ljvm -Wl,-rpath=$JAVA_HOME/jre/lib/amd64/server"
JAVA_CFLAGS="-I$JAVA_HOME/include -I$JAVA_HOME/include/linux"


# Compiler flags
GHBCI_CFLAGS="$JAVA_CFLAGS $GOBJECT_CFLAGS"
GHBCI_LIBS="$JAVA_LIBS $GOBJECT_LIBS"

AM_INIT_AUTOMAKE([-Wall -Werror foreign -Wno-portability])
AM_PATH_GLIB_2_0
AM_PROG_AR
AM_SILENT_RULES([yes])
LT_INIT

# Versioning
GHBCI_MAJOR_VERSION=ghbci_major_version
GHBCI_MINOR_VERSION=ghbci_minor_version
GHBCI_MICRO_VERSION=ghbci_micro_version
GHBCI_VERSION=ghbci_version
AC_SUBST(GHBCI_MAJOR_VERSION)
AC_SUBST(GHBCI_MINOR_VERSION)
AC_SUBST(GHBCI_MICRO_VERSION)
AC_SUBST(GHBCI_VERSION)

# Introspection
GOBJECT_INTROSPECTION_CHECK([1.38.0])

dnl Replace libtool with a wrapper that clobbers dependency_libs in *.la files
dnl http://lists.fedoraproject.org/pipermail/devel/2010-November/146343.html
LIBTOOL='bash $(top_srcdir)/libtool-kill-dependency_libs.sh $(top_builddir)/libtool'
AC_SUBST([LIBTOOL])

# Output
AC_SUBST(GHBCI_CFLAGS)
AC_SUBST(GHBCI_LIBS)

AC_CONFIG_FILES([Makefile ghbci/Makefile])

AC_OUTPUT
