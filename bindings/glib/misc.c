// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * This file is part of libgpiod.
 *
 * Copyright (C) 2019 Bartosz Golaszewski <bartekgola@gmail.com>
 */

#include <gpiod.h>
#include <gpiod-glib.h>

const gchar *g_gpiod_version_string(void)
{
	return gpiod_version_string();
}
