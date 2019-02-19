/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * This file is part of libgpiod.
 *
 * Copyright (C) 2019 Bartosz Golaszewski <bgolaszewski@baylibre.com>
 */

#ifndef __LIBGPIOD_GPIOD_GLIB_INTERNAL__
#define __LIBGPIOD_GPIOD_GLIB_INTERNAL__

#include <gpiod.h>
#include <gpiod-glib.h>

G_BEGIN_DECLS

GpiodLine *g_gpiod_line_new(struct gpiod_line *handle, GpiodChip *owner);

G_END_DECLS

#endif /* __LIBGPIOD_GPIOD_GLIB__ */
