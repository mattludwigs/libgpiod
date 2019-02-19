/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * This file is part of libgpiod.
 *
 * Copyright (C) 2019 Bartosz Golaszewski <bgolaszewski@baylibre.com>
 */

#ifndef __LIBGPIOD_GPIOD_GLIB__
#define __LIBGPIOD_GPIOD_GLIB__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

struct _GpiodChip;
typedef struct _GpiodChip GpiodChip;
struct _GpiodLine;
typedef struct _GpiodLine GpiodLine;

G_DECLARE_FINAL_TYPE(GpiodChip, g_gpiod_chip, G_GPIOD, CHIP, GObject);

#define G_GPIOD_TYPE_CHIP (g_gpiod_chip_get_type())
#define G_GPIOD_CHIP(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), G_GPIOD_TYPE_CHIP, GpiodChip))

GpiodChip *g_gpiod_chip_new(const gchar *devname, GError **err);

const gchar *g_gpiod_chip_name(GpiodChip *chip);

const gchar *g_gpiod_chip_label(GpiodChip *chip);

guint g_gpiod_chip_num_lines(GpiodChip *chip);

GList *g_gpiod_chip_list_get(GError **err);

GpiodLine *g_gpiod_chip_get_line(GpiodChip *chip, guint offset, GError **err);

GList *g_gpiod_chip_get_all_lines(GpiodChip *chip, GError **err);

G_DECLARE_FINAL_TYPE(GpiodLine, g_gpiod_line, G_GPIOD, LINE, GObject);

#define G_GPIOD_TYPE_LINE (g_gpiod_line_get_type())
#define G_GPIOD_LINE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), G_GPIOD_TYPE_LINE, GpiodLine))

guint g_gpiod_line_offset(GpiodLine *line);

const gchar *g_gpiod_line_name(GpiodLine *line);

const gchar *g_gpiod_line_consumer(GpiodLine *line);

typedef enum {
	G_GPIOD_LINE_DIRECTION_INPUT = 1,
	G_GPIOD_LINE_DIRECTION_OUTPUT,
} GpiodLineDir;

GpiodLineDir g_gpiod_line_direction(GpiodLine *line);

typedef enum {
	G_GPIOD_LINE_ACTIVE_HIGH = 1,
	G_GPIOD_LINE_ACTIVE_LOW,
} GpiodLineActive;

GpiodLineActive g_gpiod_line_active_state(GpiodLine *line);

gboolean g_gpiod_line_is_used(GpiodLine *line);

gboolean g_gpiod_line_is_open_drain(GpiodLine *line);

gboolean g_gpiod_line_is_open_source(GpiodLine *line);

GpiodChip *g_gpiod_line_get_chip(GpiodLine *line);

const gchar *g_gpiod_version_string(void);

G_END_DECLS

#endif /* __LIBGPIOD_GPIOD_GLIB__ */
