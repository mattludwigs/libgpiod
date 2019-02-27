// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * This file is part of libgpiod.
 *
 * Copyright (C) 2018-2019 Bartosz Golaszewski <bartekgola@gmail.com>
 */

#include <glib.h>
#include <gudev/gudev.h>

#include "gpio-dbus.h"

#define GPIODBUS_DAEMON_TYPE (gpiodbus_daemon_get_type())
#define GPIODBUS_DAEMON(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), \
		GPIODBUS_DAEMON_TYPE, GpioDBusDaemon))

struct _GpioDBusDaemon {
	GObject parent;

	GDBusConnection *conn;
	GUdevClient *udev;
	GDBusObjectManagerServer *manager;
	GHashTable *chips;

	gboolean listening;
};

typedef struct {
	GpioDBusChipCtrl *chip_ctrl;
	GDBusObjectManagerServer *manager;
	GList *lines;
} GpioDbusDaemonChipData;

struct _GpioDBusDaemonClass {
	GObjectClass parent;
};

typedef struct _GpioDBusDaemonClass GpioDBusDaemonClass;

G_DEFINE_TYPE(GpioDBusDaemon, gpiodbus_daemon, G_TYPE_OBJECT);

static const gchar* const gpiodbus_daemon_udev_subsystems[] = { "gpio", NULL };

static void gpiodbus_daemon_init(GpioDBusDaemon *daemon G_GNUC_UNUSED)
{
	g_debug("creating GPIO daemon");
}

GpioDBusDaemon *gpiodbus_daemon_new(void)
{
	return GPIODBUS_DAEMON(g_object_new(GPIODBUS_DAEMON_TYPE, NULL));
}

static void gpiodbus_daemon_finalize(GObject *obj)
{
	GpioDBusDaemon *daemon = GPIODBUS_DAEMON(obj);
	GError *err = NULL;
	gboolean rv;

	if (!daemon->listening)
		return;

	g_debug("destroying GPIO daemon");

	g_hash_table_unref(daemon->chips);
	g_object_unref(daemon->udev);
	g_object_unref(daemon->manager);

	g_clear_error(&err);
	rv = g_dbus_connection_close_sync(daemon->conn, NULL, &err);
	if (!rv)
		g_warning("error closing dbus connection: %s",
			  err->message);
}

static void gpiodbus_daemon_class_init(GpioDBusDaemonClass *daemon_class)
{
	GObjectClass *class = G_OBJECT_CLASS(daemon_class);

	class->finalize = gpiodbus_daemon_finalize;
}

static void gpiodbus_daemon_chip_data_free(gpointer data)
{
	GpioDbusDaemonChipData *chip_data = data;

	g_list_free_full(chip_data->lines, g_object_unref);
	g_object_unref(chip_data->chip_ctrl);
	g_object_unref(chip_data->manager);
	g_free(chip_data);
}

static void gpiodbus_daemon_export_chip_object(GpioDBusDaemon *daemon,
					       const gchar *devname)
{
	g_autofree gchar *obj_path = NULL;
	GpioDbusDaemonChipData *chip_data;
	const gchar *base_path;
	GError *err = NULL;
	GpiodChip *chip;
	GList *lines;
	gboolean rv;

	chip = g_gpiod_chip_new(devname, &err);
	if (err) {
		g_warning("unable to open the GPIO chip device: %s",
			  err->message);
		return;
	}

	lines = g_gpiod_chip_get_all_lines(chip, &err);
	if (err) {
		g_warning("unable to retrieve GPIO lines: %s",
			  err->message);
		g_object_unref(chip);
		return;
	}

	chip_data = g_malloc0(sizeof(*chip_data));

	chip_data->chip_ctrl = gpiodbus_chipctrl_new(chip, daemon->manager);
	/* FIXME free resources */
	g_return_if_fail(chip_data->chip_ctrl);

	base_path = g_dbus_object_manager_get_object_path(
				G_DBUS_OBJECT_MANAGER(daemon->manager));
	obj_path = g_strdup_printf("%s/%s", base_path,
				   g_gpiod_chip_name(chip));
	chip_data->manager = g_dbus_object_manager_server_new(obj_path);

	g_dbus_object_manager_server_set_connection(chip_data->manager,
						    daemon->conn);

	g_list_free_full(lines, g_object_unref);

	rv = g_hash_table_insert(daemon->chips, g_strdup(devname), chip_data);
	/* It's a programming bug if the chip already exists. */
	g_assert_true(rv);
}

static void gpiodbus_daemon_remove_chip_object(GpioDBusDaemon *daemon,
					       const gchar *devname)
{
	gboolean rv;

	rv = g_hash_table_remove(daemon->chips, devname);
	/* It's a programming bug if the chip didn't exist. */
	g_assert_true(rv);
}

/*
 * We get two uevents per action per gpiochip. One is for the new-style
 * character device, the other for legacy sysfs devices. We are only concerned
 * with the former, which we can tell from the latter by the presence of
 * the device file.
 */
static gboolean gpiodbus_daemon_is_gpiochip_device(GUdevDevice *dev)
{
	return g_udev_device_get_device_file(dev) != NULL;
}

static void gpiodbus_daemon_on_uevent(GUdevClient *udev G_GNUC_UNUSED,
		      const gchar *action, GUdevDevice *dev, gpointer data)
{
	GpioDBusDaemon *daemon = data;
	const gchar *devname;

	if (!gpiodbus_daemon_is_gpiochip_device(dev))
		return;

	devname = g_udev_device_get_name(dev);

	g_debug("uevent: %s action on %s device", action, devname);

	if (g_strcmp0(action, "add") == 0)
		gpiodbus_daemon_export_chip_object(daemon, devname);
	else if (g_strcmp0(action, "remove") == 0)
		gpiodbus_daemon_remove_chip_object(daemon, devname);
	else
		g_warning("unknown action for uevent: %s", action);
}

static void gpiodbus_daemon_handle_chip_dev(gpointer data, gpointer user_data)
{
	GpioDBusDaemon *daemon = user_data;
	GUdevDevice *dev = data;
	const gchar *devname;

	devname = g_udev_device_get_name(dev);

	if (gpiodbus_daemon_is_gpiochip_device(dev))
		gpiodbus_daemon_export_chip_object(daemon, devname);

	g_object_unref(dev);
}

void gpiodbus_daemon_listen(GpioDBusDaemon *daemon, GDBusConnection *conn)
{
	GList *devs;
	gulong rv;

	daemon->conn = conn;

	daemon->chips = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
					      gpiodbus_daemon_chip_data_free);

	daemon->udev = g_udev_client_new(gpiodbus_daemon_udev_subsystems);
	/* Subscribe for GPIO uevents. */
	rv = g_signal_connect(daemon->udev, "uevent",
			      G_CALLBACK(gpiodbus_daemon_on_uevent), daemon);
	g_assert_true(rv);

	daemon->manager = g_dbus_object_manager_server_new("/org/gpiod");

	devs = g_udev_client_query_by_subsystem(daemon->udev, "gpio");
	g_list_foreach(devs, gpiodbus_daemon_handle_chip_dev, daemon);
	g_list_free(devs);

	g_dbus_object_manager_server_set_connection(daemon->manager,
						    daemon->conn);

	daemon->listening = TRUE;
}
