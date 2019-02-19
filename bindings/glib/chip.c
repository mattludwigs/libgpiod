// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * This file is part of libgpiod.
 *
 * Copyright (C) 2019 Bartosz Golaszewski <bartekgola@gmail.com>
 */

#include <errno.h>
#include <gio/gio.h>
#include <glib.h>
#include <gpiod.h>
#include <gpiod-glib.h>

#include "internal.h"

struct _GpiodChip {
	GObject parent;

	struct gpiod_chip *handle;
};

G_DEFINE_TYPE(GpiodChip, g_gpiod_chip, G_TYPE_OBJECT);

static void g_gpiod_chip_init(GpiodChip *chip G_GNUC_UNUSED)
{

}

static void g_gpiod_chip_finalize(GObject *obj)
{
	GpiodChip *chip = G_GPIOD_CHIP(obj);

	gpiod_chip_close(chip->handle);
}

static void g_gpiod_chip_class_init(GpiodChipClass *chip_class)
{
	GObjectClass *class = G_OBJECT_CLASS(chip_class);

	class->finalize = g_gpiod_chip_finalize;
}

static GpiodChip *g_gpiod_chip_new_from_handle(struct gpiod_chip *handle)
{
	GpiodChip *chip;

	chip = G_GPIOD_CHIP(g_object_new(G_GPIOD_TYPE_CHIP, NULL));
	chip->handle = handle;

	return chip;
}

GpiodChip *g_gpiod_chip_new(const gchar *devname, GError **err)
{
	struct gpiod_chip *handle;

	g_assert(devname);

	handle = gpiod_chip_open_lookup(devname);
	if (!handle) {
		g_set_error(err, G_IO_ERROR, g_io_error_from_errno(errno),
			    "unable to open GPIO chip '%s': %s", devname,
			    g_strerror(errno));
		return NULL;
	}

	return g_gpiod_chip_new_from_handle(handle);
}

const gchar *g_gpiod_chip_name(GpiodChip *chip)
{
	g_assert(chip);

	return gpiod_chip_name(chip->handle);
}

const gchar *g_gpiod_chip_label(GpiodChip *chip)
{
	g_assert(chip);

	return gpiod_chip_label(chip->handle);
}

guint g_gpiod_chip_num_lines(GpiodChip *chip)
{
	g_assert(chip);

	return gpiod_chip_num_lines(chip->handle);
}

GList *g_gpiod_chip_list_get(GError **err)
{
	struct gpiod_chip_iter *iter;
	struct gpiod_chip *handle;
	GList *chips = NULL;
	GpiodChip *chip;

	iter = gpiod_chip_iter_new();
	if (!iter) {
		g_set_error(err, G_IO_ERROR, g_io_error_from_errno(errno),
			    "unable to create a GPIO chip iterator: %s",
			    g_strerror(errno));
		return NULL;
	}

	gpiod_foreach_chip_noclose(iter, handle) {
		chip = g_gpiod_chip_new_from_handle(handle);
		chips = g_list_append(chips, chip);
	}

	gpiod_chip_iter_free(iter);

	return chips;
}

GpiodLine *g_gpiod_chip_get_line(GpiodChip *chip, guint offset, GError **err)
{
	struct gpiod_line *handle;
	GpiodLine *line;

	g_assert(chip);

	handle = gpiod_chip_get_line(chip->handle, offset);
	if (!handle) {
		g_set_error(err, G_IO_ERROR, g_io_error_from_errno(errno),
			    "unable to retrieve the GPIO line at offset %u: %s",
			    offset, g_strerror(errno));
		return NULL;
	}

	line = g_gpiod_line_new(handle, chip);

	return line;
}

GList *g_gpiod_chip_get_all_lines(GpiodChip *chip, GError **err)
{
	GError *new_err = NULL;
	GList *lines = NULL;
	guint i, num_lines;
	GpiodLine *line;

	g_assert(chip);

	num_lines = g_gpiod_chip_num_lines(chip);

	for (i = 0; i < num_lines; i++) {
		line = g_gpiod_chip_get_line(chip, i, &new_err);
		if (new_err) {
			g_propagate_error(err, new_err);
			g_list_free_full(lines, g_object_unref);
			return NULL;
		}

		lines = g_list_append(lines, line);
	}

	return lines;
}
