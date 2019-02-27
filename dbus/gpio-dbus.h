// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * This file is part of libgpiod.
 *
 * Copyright (C) 2018-2019 Bartosz Golaszewski <bartekgola@gmail.com>
 */

#ifndef __LIBGPIOD_GPIO_DBUS_H__
#define __LIBGPIOD_GPIO_DBUS_H__

#include <gio/gio.h>
#include <gpiod-glib.h>

struct _GpioDBusDaemon;
typedef struct _GpioDBusDaemon GpioDBusDaemon;

GpioDBusDaemon *gpiodbus_daemon_new(void);

void gpiodbus_daemon_listen(GpioDBusDaemon *daemon, GDBusConnection *conn);

struct _GpioDBusChipCtrl;
typedef struct _GpioDBusChipCtrl GpioDBusChipCtrl;

GpioDBusChipCtrl *gpiodbus_chipctrl_new(GpiodChip *chip,
					GDBusObjectManagerServer *manager);

struct _GpioDBusLineCtrl;
typedef struct _GpioDBusLineCtrl GpioDBusLineCtrl;

GpioDBusLineCtrl *gpiodbus_linectrl_new(GpiodLine *line,
					GDBusObjectManagerServer *manager);

#endif /* __LIBGPIOD_GPIO_DBUS_H__ */
