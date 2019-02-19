# SPDX-License-Identifier: LGPL-2.1-or-later

#
# This file is part of libgpiod.
#
# Copyright (C) 2019 Bartosz Golaszewski <bartekgola@gmail.com>
#

AM_CFLAGS = -I$(top_srcdir)/bindings/glib/ -include $(top_builddir)/config.h
AM_CFLAGS += -Wall -Wextra -g $(GLIB_CFLAGS) $(GIO_CFLAGS) $(GIO_UNIX_CFLAGS)
AM_LDFLAGS = -lgpiod-glib -L$(top_builddir)/bindings/glib
AM_LDFLAGS += $(GLIB_LIBS) $(GIO_LIBS) $(GIO_UNIX_LIBS)

check_PROGRAMS = gpiodetect-glib gpioinfo-glib

gpiodetect_glib_SOURCES = gpiodetect-glib.c

gpioinfo_glib_SOURCES = gpioinfo-glib.c
