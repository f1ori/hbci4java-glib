# preamble

NULL =
AM_CPPFLAGS =
AM_CFLAGS =
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =
DISTCLEANFILES =
lib_LTLIBRARIES =
bin_PROGRAMS =


AM_CPPFLAGS += \
	$(GHBCI_CFLAGS) \
	$(NULL)

AM_CFLAGS += \
	$(GHBCI_CFLAGS) \
	$(NULL)


ghbci-marshal.h: marshal.list
	-glib-genmarshal --header --prefix=ghbci_marshal marshal.list > $@
ghbci-marshal.c: marshal.list
	-glib-genmarshal --body --prefix=ghbci_marshal marshal.list > $@
BUILT_SOURCES += ghbci-marshal.c ghbci-marshal.h
CLEANFILES += ghbci-marshal.c ghbci-marshal.h


source_public_h = \
	$(top_srcdir)/ghbci/ghbci-statement.h \
	$(top_srcdir)/ghbci/ghbci-account.h \
	$(top_srcdir)/ghbci/ghbci-context.h \
	$(NULL)

source_private_h = \
	$(top_srcdir)/ghbci/ghbci-statement-private.h \
	$(top_srcdir)/ghbci/ghbci-account-private.h \
	$(top_srcdir)/ghbci/ghbci-marshal.h \
	$(NULL)

source_c = \
	ghbci-statement.c \
	ghbci-account.c \
	ghbci-context.c \
	$(NULL)

lib_LTLIBRARIES += libghbci-0.1.la
libghbci_0_1_la_LIBADD = $(GHBCI_LIBS)
libghbci_0_1_la_SOURCES = $(source_c) $(source_public_h) $(source_private_h) ghbci-marshal.c ghbci-marshal.h

ghbciincludedir = $(includedir)/ghbci
ghbciinclude_DATA = \
	$(source_public_h) \
	$(source_private_h) \
	$(NULL)


jardir = $(pkgdatadir)
jar_DATA = hbci4java.jar
EXTRA_DIST += hbci4java.jar test.py


-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = GHbci_0_1_gir
INTROSPECTION_SCANNER_ARGS = --symbol-prefix=ghbci --add-include-path=$(srcdir) --warn-all --c-include='ghbci.h'
INTROSPECTION_COMPILER_ARGS =

if HAVE_INTROSPECTION
introspection_sources = $(source_c) $(source_public_h) ghbci-marshal.c ghbci-marshal.h

GHbci-0.1.gir: libghbci-0.1.la
GHbci_0_1_gir_INCLUDES = GObject-2.0
GHbci_0_1_gir_CFLAGS = $(AM_CPPFLAGS)
GHbci_0_1_gir_PACKAGES =
GHbci_0_1_gir_LIBS = libghbci-0.1.la
GHbci_0_1_gir_FILES = $(introspection_sources)
GHbci_0_1_gir_NAMESPACE = GHbci
GHbci_0_1_gir_VERSION = 0.1
INTROSPECTION_GIRS += GHbci-0.1.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif

