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

#define GPIODBUS_CHIPCTRL_TYPE (gpiodbus_chipctrl_get_type())
#define GPIODBUS_CHIPCTRL(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), \
		GPIODBUS_CHIPCTRL_TYPE, GpioDBusChipCtrl))

struct _GpioDBusChipCtrl {
	GObject parent;

	GpiodChip *chip;
	GDBusObjectManagerServer *manager;
	gchar *obj_path;
};

struct _GpioDBusChipCtrlClass {
	GObjectClass parent;
};

typedef struct _GpioDBusChipCtrlClass GpioDBusChipCtrlClass;

G_DEFINE_TYPE(GpioDBusChipCtrl, gpiodbus_chipctrl, G_TYPE_OBJECT);

static void gpiodbus_chipctrl_init(GpioDBusChipCtrl *chip G_GNUC_UNUSED)
{

}

GpioDBusChipCtrl *gpiodbus_chipctrl_new(GpiodChip *chip,
					GDBusObjectManagerServer *manager)
{
	GpioDBusObjectSkeleton *obj_skeleton;
	GpioDBusChipCtrl *chip_ctrl;
	GpioDBusChip *chip_obj;
	const gchar *base_path;

	g_debug("creating a dbus object for GPIO chip: %s",
		g_gpiod_chip_name(chip));

	chip_ctrl = GPIODBUS_CHIPCTRL(g_object_new(GPIODBUS_CHIPCTRL_TYPE,
						   NULL));
	chip_ctrl->chip = chip;

	base_path = g_dbus_object_manager_get_object_path(
					G_DBUS_OBJECT_MANAGER(manager));

	chip_ctrl->obj_path = g_strdup_printf("%s/%s", base_path,
					      g_gpiod_chip_name(chip));
	obj_skeleton = gpio_dbus_object_skeleton_new(chip_ctrl->obj_path);

	chip_obj = gpio_dbus_chip_skeleton_new();
	gpio_dbus_object_skeleton_set_chip(obj_skeleton, chip_obj);
	g_object_unref(chip_obj);

	gpio_dbus_chip_set_name(chip_obj, g_gpiod_chip_name(chip));
	gpio_dbus_chip_set_label(chip_obj, g_gpiod_chip_label(chip));
	gpio_dbus_chip_set_num_lines(chip_obj, g_gpiod_chip_num_lines(chip));

	g_dbus_object_manager_server_export(manager,
					G_DBUS_OBJECT_SKELETON(obj_skeleton));
	g_object_unref(obj_skeleton);

	g_object_ref(manager);
	chip_ctrl->manager = manager;

	return chip_ctrl;
}

static void gpiodbus_chipctrl_finalize(GObject *obj)
{
	GpioDBusChipCtrl *chip_ctrl = GPIODBUS_CHIPCTRL(obj);

	g_debug("destrying dbus object for GPIO chip %s",
		g_gpiod_chip_name(chip_ctrl->chip));

	g_dbus_object_manager_server_unexport(chip_ctrl->manager,
					      chip_ctrl->obj_path);
	g_object_unref(chip_ctrl->manager);
	g_free(chip_ctrl->obj_path);
	g_object_unref(chip_ctrl->chip);
}

static void gpiodbus_chipctrl_class_init(GpioDBusChipCtrlClass *chipctrl_class)
{
	GObjectClass *class = G_OBJECT_CLASS(chipctrl_class);

	class->finalize = gpiodbus_chipctrl_finalize;
}
