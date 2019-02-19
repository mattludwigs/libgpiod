// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * This file is part of libgpiod.
 *
 * Copyright (C) 2019 Bartosz Golaszewski <bartekgola@gmail.com>
 */

#include <glib.h>
#include <glib-object.h>
#include <glib/gprintf.h>
#include <gpiod-glib.h>
#include <stdio.h>
#include <stdlib.h>

static void print_line_info(gpointer data, gpointer user_data G_GNUC_UNUSED)
{
	const gchar *name, *consumer;
	GpiodLine *line = data;
	GpiodLineDir direction;
	GpiodLineActive active;
	guint offset;

	offset = g_gpiod_line_offset(line);
	name = g_gpiod_line_name(line);
	consumer = g_gpiod_line_consumer(line);
	direction = g_gpiod_line_direction(line);
	active = g_gpiod_line_active_state(line);

	g_printf("\tline %3u %12s %12s %8s %10s\n",
		 offset,
		 name ?: "unnamed",
		 consumer ?: "unused",
		 direction == G_GPIOD_LINE_DIRECTION_INPUT ? "input"
							   : "output",
		 active == G_GPIOD_LINE_ACTIVE_LOW ? "active-low"
						   : "active-high");
}

static void print_chip_info(gpointer data, gpointer user_data)
{
	GError **err = user_data, *new_err = NULL;
	GpiodChip *chip = data;
	GList *lines;

	lines = g_gpiod_chip_get_all_lines(chip, &new_err);
	if (new_err) {
		g_propagate_error(err, new_err);
		return;
	}

	g_printf("%s - %u lines:\n",
		 g_gpiod_chip_name(chip), g_gpiod_chip_num_lines(chip));

	g_list_foreach(lines, print_line_info, NULL);
	g_list_free_full(lines, g_object_unref);
}

int main(int argc G_GNUC_UNUSED, char **argv G_GNUC_UNUSED)
{
	GError *err = NULL;
	GList *chips;

	chips = g_gpiod_chip_list_get(&err);
	if (err)
		goto err_out;

	g_list_foreach(chips, print_chip_info, &err);
	g_list_free_full(chips, g_object_unref);
	if (err)
		goto err_out;

	return EXIT_SUCCESS;

err_out:
	g_error("unable to print GPIO chip info: %s", err->message);
	g_error_free(err);

	return EXIT_FAILURE;
}
