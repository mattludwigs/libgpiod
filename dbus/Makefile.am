# SPDX-License-Identifier: LGPL-2.1-or-later

#
# This file is part of libgpiod.
#
# Copyright (C) 2018-2019 Bartosz Golaszewski <bartekgola@gmail.com>
#

SUBDIRS = . data examples

AM_CFLAGS = -I$(top_srcdir)/bindings/glib/ -include $(top_builddir)/config.h
AM_CFLAGS += -Wall -Wextra -g $(GLIB_CFLAGS) $(GIO_CFLAGS) $(GIO_UNIX_CFLAGS) $(GUDEV_CFLAGS)
AM_LDFLAGS = $(GLIB_LIBS) $(GIO_LIBS) $(GIO_UNIX_LIBS) $(GUDEV_LIBS)

LDADD = $(top_builddir)/bindings/glib/libgpiod-glib.la

bin_PROGRAMS = gpio-dbus

generated-gpio-dbus.h generated-gpio-dbus.c: data/org.gpiod.xml
	@gdbus-codegen \
		--interface-prefix org.gpiod \
		--c-namespace GpioDBus \
		--generate-c-code generated-gpio-dbus \
		--c-generate-object-manager \
		data/org.gpiod.xml

gpio_dbus_SOURCES = chip.c daemon.c gpio-dbus.c gpio-dbus.h line.c
nodist_gpio_dbus_SOURCES = generated-gpio-dbus.c generated-gpio-dbus.h
BUILT_SOURCES = $(nodist_gpio_dbus_SOURCES)
CLEANFILES = $(nodist_gpio_dbus_SOURCES)
