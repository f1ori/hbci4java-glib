JAVA_HOME=/usr/lib/jvm/java

JAVA_LIBS=-L$(JAVA_HOME)/jre/lib/amd64/server -ljvm -Wl,-rpath=$(JAVA_HOME)/jre/lib/amd64/server

JAVA_CFLAGS=-I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux


CFLAGS=-g -Wall -O2 -fPIC $(JAVA_CFLAGS) $(shell pkg-config --cflags glib-2.0 gobject-2.0)

LIBS=$(JAVA_LIBS) $(shell pkg-config --libs glib-2.0 gobject-2.0)


all: libghbci.so GHbci-0.1.typelib

libghbci.so: ghbci-context.o ghbci-account.o ghbci-statement.o ghbci-marshal.o
	gcc $(LIBS) -shared -o libghbci.so ghbci-marshal.o ghbci-context.o ghbci-account.o ghbci-statement.o

ghbci-context.o:ghbci-context.c ghbci-context.h ghbci-statement.h ghbci-marshal.h
	gcc $(CFLAGS) -c ghbci-context.c

ghbci-account.o:ghbci-account.c ghbci-account.h ghbci-context-private.h
	gcc $(CFLAGS) -c ghbci-account.c

ghbci-statement.o:ghbci-statement.c ghbci-statement.h ghbci-context-private.h
	gcc $(CFLAGS) -c ghbci-statement.c

ghbci-marshal.o:ghbci-marshal.c ghbci-marshal.h
	gcc $(CFLAGS) -c ghbci-marshal.c

GHbci-0.1.gir:libghbci.so ghbci-context.c ghbci-context.h ghbci-account.h ghbci-account.c ghbci-statement.h ghbci-statement.c ghbci-marshal.c ghbci-marshal.h
	g-ir-scanner --warn-all ghbci-context.[ch] ghbci-account.[ch] ghbci-statement.[ch] ghbci-marshal.[ch] --symbol-prefix=ghbci --library=ghbci -L`pwd` `pkg-config --cflags gobject-2.0` --include=GObject-2.0 --namespace=GHbci --nsversion=0.1 --c-include='ghbci.h' --output=GHbci-0.1.gir

GHbci-0.1.typelib:GHbci-0.1.gir
	g-ir-compiler GHbci-0.1.gir > GHbci-0.1.typelib

ghbci-marshal.h: marshal.list
	glib-genmarshal --header --prefix=ghbci_marshal marshal.list > $@
ghbci-marshal.c: marshal.list
	glib-genmarshal --body --prefix=ghbci_marshal marshal.list > $@


clean:
	rm -rf *.o

distclean: clean
	rm -rf libghbci.so GHbci-0.1.gir GHbci-0.1.typelib

test:all
	GI_TYPELIB_PATH=. LD_LIBRARY_PATH=. python test.py


# CLASSPATH=hbci4java.jar javap -s -p org.kapott.hbci.callback.HBCICallbackConsole
