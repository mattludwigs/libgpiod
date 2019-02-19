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

static void print_chip_info(gpointer data, gpointer user_data G_GNUC_UNUSED)
{
	GpiodChip *chip = data;

	g_printf("%s [%s] (%u lines)\n",
		 g_gpiod_chip_name(chip),
		 g_gpiod_chip_label(chip),
		 g_gpiod_chip_num_lines(chip));
}

int main(int argc G_GNUC_UNUSED, char **argv G_GNUC_UNUSED)
{
	GError *err = NULL;
	GList *chips;

	chips = g_gpiod_chip_list_get(&err);
	if (err) {
		g_error("unable to list GPIO chips: %s", err->message);
		g_error_free(err);
		return EXIT_FAILURE;
	}

	g_list_foreach(chips, print_chip_info, NULL);
	g_list_free_full(chips, g_object_unref);

	return EXIT_SUCCESS;
}
