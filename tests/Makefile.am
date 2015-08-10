AM_CPPFLAGS = \
	$(GHBCI_CFLAGS) \
	$(NULL)

AM_CFLAGS = \
	$(GHBCI_CFLAGS) \
	$(NULL)

LDADD =\
	$(top_builddir)/ghbci/libghbci-0.1.la \
	$(GHBCI_LIBS) \
	$(NULL)

check_PROGRAMS = test-statement

TESTS = test-statement

check_c_sources = test-statement.c
