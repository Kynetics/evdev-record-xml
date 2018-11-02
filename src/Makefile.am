version_script = $(srcdir)/libevemu.ver

lib_LTLIBRARIES = libevemu.la libevdevxml.la

libevemu_la_LDFLAGS = \
	-version-info @LIB_VERSION@ \
	-Wl,-z,defs -Wl,--as-needed \
	-Wl,--version-script=$(version_script)

libevdevxml_la_LDFLAGS = \
	-Wl,-z,defs -Wl,--as-needed

libevemu_la_SOURCES = \
	../include/linux/input.h \
	../include/linux/uinput.h \
	../include/linux/input-event-codes.h \
	evemu-impl.h \
	evemu.c \
	evemu.h \
	version.h

libevdevxml_la_SOURCES = \
	../include/linux/input.h \
	../include/linux/uinput.h \
	../include/linux/input-event-codes.h \
	evdev-xml-impl.h \
	evdev-xml.h \
	evdev-xml.c

libevemu_la_LIBADD = $(LIBEVDEV_LIBS)
libevdevxml_la_LIBADD = $(XML_LIBS) $(LIBEVDEV_LIBS)
libevdevxml_la_CFLAGS = $(XML_CFLAGS)

AM_CPPFLAGS = -I$(top_srcdir)/include/ $(LIBEVDEV_CFLAGS)

libevemuincludedir = $(includedir)
libevemuinclude_HEADERS = evemu.h

libevdevxmlincludedir = $(includedir)
libevdevxmlinclude_HEADERS = evdev-xml.h

EXTRA_DIST = $(version_script) convert-old-dumps-to-1.1.py
