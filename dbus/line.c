// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * This file is part of libgpiod.
 *
 * Copyright (C) 2018-2019 Bartosz Golaszewski <bartekgola@gmail.com>
 */

#include <errno.h>
#include <glib.h>
#include <gpiod-glib.h>
#include <gudev/gudev.h>

#include "gpio-dbus.h"
#include "generated-gpio-dbus.h"

#define GPIODBUS_LINECTRL_TYPE (gpiodbus_linectrl_get_type())
#define GPIODBUS_LINECTRL(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), \
		GPIODBUS_LINECTRL_TYPE, GpioDBusLineCtrl))

struct _GpioDBusLineCtrl {
	GObject parent;

	GpiodLine *line;
	GDBusObjectManagerServer *manager;
	gchar *obj_path;
};

struct _GpioDBusLineCtrlClass {
	GObjectClass parent;
};

typedef struct _GpioDBusLineCtrlClass GpioDBusLineCtrlClass;

G_DEFINE_TYPE(GpioDBusLineCtrl, gpiodbus_linectrl, G_TYPE_OBJECT);

static void gpiodbus_linectrl_init(GpioDBusLineCtrl *line G_GNUC_UNUSED)
{

}

GpioDBusLineCtrl *gpiodbus_linectrl_new(GpiodLine *line,
					GDBusObjectManagerServer *manager)
{
	GpioDBusObjectSkeleton *obj_skeleton;
	GpioDBusLineCtrl *line_ctrl;
	GpioDBusLine *line_obj;
	const gchar *base_path;
	GpiodChip *chip;

	chip = g_gpiod_line_get_chip(line);

	g_debug("creating a dbus object for GPIO line: chip - %s, offset - %u",
		g_gpiod_chip_name(chip), g_gpiod_line_offset(line));

	line_ctrl = GPIODBUS_LINECTRL(g_object_new(GPIODBUS_LINECTRL_TYPE,
						   NULL));
	line_ctrl->line = line;

	base_path = g_dbus_object_manager_get_object_path(
					G_DBUS_OBJECT_MANAGER(manager));

	line_ctrl->obj_path = g_strdup_printf("%s/%s/%u", base_path,
					      g_gpiod_chip_name(chip),
					      g_gpiod_line_offset(line));
	g_object_unref(chip);
	obj_skeleton = gpio_dbus_object_skeleton_new(line_ctrl->obj_path);

	line_obj = gpio_dbus_line_skeleton_new();
	gpio_dbus_object_skeleton_set_line(obj_skeleton, line_obj);
	g_object_unref(line_obj);

	gpio_dbus_line_set_offset(line_obj, g_gpiod_line_offset(line));
	gpio_dbus_line_set_name(line_obj, g_gpiod_line_name(line));
	gpio_dbus_line_set_consumer(line_obj, g_gpiod_line_consumer(line));

	g_dbus_object_manager_server_export(manager,
				G_DBUS_OBJECT_SKELETON(obj_skeleton));
	g_object_unref(obj_skeleton);

	g_object_ref(manager);
	line_ctrl->manager = manager;

	return line_ctrl;
}

static void gpiodbus_linectrl_finalize(GObject *obj)
{
	GpioDBusLineCtrl *line_ctrl;
	GpiodChip *chip;

	line_ctrl = GPIODBUS_LINECTRL(obj);
	chip = g_gpiod_line_get_chip(line_ctrl->line);

	g_debug("destrying dbus object for GPIO line: chip - %s, offset - %u",
		g_gpiod_chip_name(chip), g_gpiod_line_offset(line_ctrl->line));

	g_dbus_object_manager_server_unexport(line_ctrl->manager,
					      line_ctrl->obj_path);
	g_object_unref(line_ctrl->manager);
	g_free(line_ctrl->obj_path);
	g_object_unref(line_ctrl->line);
}

static void gpiodbus_linectrl_class_init(GpioDBusLineCtrlClass *linectrl_class)
{
	GObjectClass *class = G_OBJECT_CLASS(linectrl_class);

	class->finalize = gpiodbus_linectrl_finalize;
}
