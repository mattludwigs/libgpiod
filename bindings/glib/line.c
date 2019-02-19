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

struct _GpiodLine {
	GObject parent;

	struct gpiod_line *handle;
	GpiodChip *owner;
};

G_DEFINE_TYPE(GpiodLine, g_gpiod_line, G_TYPE_OBJECT);

static void g_gpiod_line_init(GpiodLine *chip G_GNUC_UNUSED)
{

}

static void g_gpiod_line_finalize(GObject *obj)
{
	GpiodLine *line = G_GPIOD_LINE(obj);

	g_object_unref(line->owner);
}

static void g_gpiod_line_class_init(GpiodLineClass *line_class)
{
	GObjectClass *class = G_OBJECT_CLASS(line_class);

	class->finalize = g_gpiod_line_finalize;
}

GpiodLine *g_gpiod_line_new(struct gpiod_line *handle, GpiodChip *owner)
{
	GpiodLine *line = G_GPIOD_LINE(g_object_new(G_GPIOD_TYPE_LINE, NULL));

	g_assert(handle);

	line->handle = handle;
	line->owner = owner;
	g_object_ref(owner);

	return line;
}

guint g_gpiod_line_offset(GpiodLine *line)
{
	g_assert(line);

	return gpiod_line_offset(line->handle);
}

const gchar *g_gpiod_line_name(GpiodLine *line)
{
	g_assert(line);

	return gpiod_line_name(line->handle);
}

const gchar *g_gpiod_line_consumer(GpiodLine *line)
{
	g_assert(line);

	return gpiod_line_consumer(line->handle);
}

GpiodLineDir g_gpiod_line_direction(GpiodLine *line)
{
	gint direction;

	g_assert(line);

	direction = gpiod_line_direction(line->handle);
	switch (direction) {
	case GPIOD_LINE_DIRECTION_INPUT:
		return G_GPIOD_LINE_DIRECTION_INPUT;
	case GPIOD_LINE_DIRECTION_OUTPUT:
		return G_GPIOD_LINE_DIRECTION_OUTPUT;
	};

	g_assert_not_reached();
}

GpiodLineActive g_gpiod_line_active_state(GpiodLine *line)
{
	gint active;

	g_assert(line);

	active = gpiod_line_active_state(line->handle);
	switch (active) {
	case GPIOD_LINE_ACTIVE_STATE_HIGH:
		return G_GPIOD_LINE_ACTIVE_HIGH;
	case GPIOD_LINE_ACTIVE_STATE_LOW:
		return G_GPIOD_LINE_ACTIVE_LOW;
	};

	g_assert_not_reached();
}

gboolean g_gpiod_line_is_used(GpiodLine *line)
{
	g_assert(line);

	return gpiod_line_is_used(line->handle);
}

gboolean g_gpiod_line_is_open_drain(GpiodLine *line)
{
	g_assert(line);

	return gpiod_line_is_open_drain(line->handle);
}

gboolean g_gpiod_line_is_open_source(GpiodLine *line)
{
	g_assert(line);

	return gpiod_line_is_open_source(line->handle);
}

GpiodChip *g_gpiod_line_get_chip(GpiodLine *line)
{
	g_assert(line);

	g_object_ref(line->owner);
	return line->owner;
}
