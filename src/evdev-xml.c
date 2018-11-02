/*****************************************************************************
 *
 * evdev - Kernel device emulation
 *
 * Copyright (C) 2018 Kynetics, LLC
 * Copyright (C) 2010-2012 Canonical Ltd.
 * Copyright (C) 2009, 2013-2015 Red Hat, Inc.
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2010 Henrik Rydberg <rydberg@euromail.se>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#define _GNU_SOURCE

#include "evdev-xml-impl.h"
#include <libxml/xmlwriter.h>
#include <poll.h>

#define SYSCALL(call) while (((call) == -1) && (errno == EINTR))

#define DEF_ENCODING "ISO-8859-1"

/* Define helper macros for libxml api */
#define start(writer, ...) xmlTextWriterStartElement(writer, __VA_ARGS__)
#define end(writer, dummy) xmlTextWriterEndElement(writer)
#define attribute(writer, ...) xmlTextWriterWriteAttribute(writer, __VA_ARGS__)
#define format(writer, ...) xmlTextWriterWriteFormatString(writer, __VA_ARGS__)

#define ID_GETTER(field)                                                     \
	static unsigned int evdev_get_id_##field(const struct evdev_device *dev) \
	{                                                                        \
		return libevdev_get_id_##field(dev->evdev);                          \
	}

ID_GETTER(bustype)
ID_GETTER(version)
ID_GETTER(product)
ID_GETTER(vendor)

#define ABS_GETTER(field)                                                      \
	static int evdev_get_abs_##field(const struct evdev_device *dev, int code) \
	{                                                                          \
		return libevdev_get_abs_##field(dev->evdev, code);                     \
	}

ABS_GETTER(minimum)
ABS_GETTER(maximum)
ABS_GETTER(fuzz)
ABS_GETTER(flat)
ABS_GETTER(resolution)

void evdev_dev_set_name(struct evdev_device *dev, const char *name)
{
	if (name)
		libevdev_set_name(dev->evdev, name);
}

struct evdev_device *evdev_dev_new(const char *name)
{
	struct evdev_device *dev = calloc(1, sizeof(struct evdev_device));

	if (dev) {
		dev->evdev = libevdev_new();
		if (!dev->evdev) {
			free(dev);
			return NULL;
		}
		evdev_dev_set_name(dev, name);
	}

	return dev;
}

void evdev_dev_delete(struct evdev_device *dev)
{
	if (dev == NULL)
		return;

	if (dev->uidev)
		evdev_dev_destroy(dev);
	libevdev_free(dev->evdev);
	free(dev);
}

int evdev_dev_extract(struct evdev_device *dev, int fd)
{
	if (libevdev_get_fd(dev->evdev) != -1) {
		libevdev_free(dev->evdev);
		dev->evdev = libevdev_new();
		if (!dev->evdev)
			return -ENOMEM;
	}
	return libevdev_set_fd(dev->evdev, fd);
}

void evdev_dev_destroy(struct evdev_device *dev)
{
	if (dev->uidev) {
		libevdev_uinput_destroy(dev->uidev);
		dev->uidev = NULL;
	}
}

static void evdev_xml_print_event(struct xml_writer *wr_handle, const struct input_event *ev)
{
	char buffer[64];

	start(wr_handle->writer, "event");

	/* sec */
	sprintf(buffer, "%ld", ev->input_event_sec);
	attribute(wr_handle->writer, "sec", buffer);
	/* usec */
	sprintf(buffer, "%ld", (unsigned)ev->input_event_usec);
	attribute(wr_handle->writer, "usec", buffer);
	/* type */
	sprintf(buffer, "%s", libevdev_event_type_get_name(ev->type));
	attribute(wr_handle->writer, "type", buffer);
	/* code */
	sprintf(buffer, "%s", libevdev_event_code_get_name(ev->type, ev->code));
	attribute(wr_handle->writer, "code", buffer);
	/* value */
	sprintf(buffer, "%ld", ev->value);
	attribute(wr_handle->writer, "value", buffer);

	end(wr_handle->writer, "event");

	return;
}

struct xml_writer *evdev_xml_writer_new()
{
	struct xml_writer *wr_handle = calloc(1, sizeof(struct xml_writer));

	if(!wr_handle)
		return NULL;
	return wr_handle;
}

int evdev_xml_init(struct xml_writer *wr_handle, const char *outfile)
{
	int ret;

	if (!wr_handle)
		return -1;

	if (!outfile)
		return -1;

	LIBXML_TEST_VERSION

	wr_handle->writer = xmlNewTextWriterFilename(outfile, 0);
	if (!wr_handle->writer)
		return -1;

	xmlTextWriterSetIndent(wr_handle->writer, 1);
	xmlTextWriterSetIndentString(wr_handle->writer, BAD_CAST "    ");

	ret = xmlTextWriterStartDocument(wr_handle->writer, NULL, DEF_ENCODING, NULL);
	if (ret < 0)
		return ret;

	return start(wr_handle->writer, "evdev-capture");
}

int evdev_xml_close(struct xml_writer *wr_handle)
{
	int ret;

	if (!wr_handle->writer)
		return -1;

	ret = xmlTextWriterEndDocument(wr_handle->writer);
	if (ret < 0)
		return ret;

	xmlCleanupParser();

	return end(wr_handle->writer, "evdev-capture");
}

int evdev_xml_print_dev_info(struct xml_writer *wr_handle, const struct evdev_device *dev)
{
	int i, j;
	int state;
	char buffer[64];

	if(!wr_handle->writer)
		return -1;

	start(wr_handle->writer, "info");

	start(wr_handle->writer, "id");
	/* bus */
	sprintf(buffer, "%#x", evdev_get_id_bustype(dev));
	attribute(wr_handle->writer, "bus", buffer);
	/* vendor */
	sprintf(buffer, "%#x", evdev_get_id_vendor(dev));
	attribute(wr_handle->writer, "vendor", buffer);
	/* product */
	sprintf(buffer, "%#x", evdev_get_id_product(dev));
	attribute(wr_handle->writer, "product", buffer);
	/* version */
	sprintf(buffer, "%#x", evdev_get_id_version(dev));
	attribute(wr_handle->writer, "version", buffer);
	/* name */
	format(wr_handle->writer, "%s", libevdev_get_name(dev->evdev));
	end(wr_handle->writer, "id");

	for (i = 0; i < EV_CNT; i++)
	{
		if (!libevdev_has_event_type(dev->evdev, i))
			continue;

		start(wr_handle->writer, "event-type");
		/* type */
		sprintf(buffer, "%s", libevdev_event_type_get_name(i));
		attribute(wr_handle->writer, "type", buffer);

		for (j = 0; j <= libevdev_event_type_get_max(i); j++)
		{
			if (!libevdev_has_event_code(dev->evdev, i, j))
				continue;

			if(!libevdev_event_code_get_name(i, j))
				continue;

			start(wr_handle->writer, "code");
			/* value */
			sprintf(buffer, "%s", libevdev_event_code_get_name(i, j));
			attribute(wr_handle->writer, "value", buffer);

			switch (i)
			{
			case EV_ABS:
				/* abs-value */
				sprintf(buffer, "%d", libevdev_get_event_value(dev->evdev, EV_ABS, j));
				attribute(wr_handle->writer, "abs-value", buffer);
				/* abs-min */
				sprintf(buffer, "%d", evdev_get_abs_minimum(dev, j));
				attribute(wr_handle->writer, "abs-min", buffer);
				/* abs-max */
				sprintf(buffer, "%d", evdev_get_abs_maximum(dev, j));
				attribute(wr_handle->writer, "abs-max", buffer);
				/* abs-fuzz */
				sprintf(buffer, "%d", evdev_get_abs_fuzz(dev, j));
				attribute(wr_handle->writer, "abs-fuzz", buffer);
				/* abs-flat */
				sprintf(buffer, "%d", evdev_get_abs_flat(dev, j));
				attribute(wr_handle->writer, "abs-flat", buffer);
				/* abs-resolution */
				sprintf(buffer, "%d", evdev_get_abs_resolution(dev, j));
				attribute(wr_handle->writer, "abs-resolution", buffer);

				break;
			case EV_LED:
			case EV_SW:
				state = libevdev_get_event_value(dev->evdev, i, j);
				/* state */
				sprintf(buffer, "%d", state);
				attribute(wr_handle->writer, "state", buffer);
				break;
			default:
				break;
			}

			end(wr_handle->writer, "code");
		}

		end(wr_handle->writer, "event-type");
	}

	end(wr_handle->writer, "info");

	return 0;
}

int evdev_xml_record_dev(struct xml_writer *wr_handle, int fd, int ms)
{
	struct pollfd fds = {fd, POLLIN, 0};
	struct input_event ev;
	int ret;
	long offset = 0;

	start(wr_handle->writer, "events");

	while (poll(&fds, 1, ms) > 0)
	{
		SYSCALL(ret = read(fd, &ev, sizeof(ev)));
		if (ret < 0)
			return ret;
		if (ret == sizeof(ev))
		{
			evdev_xml_print_event(wr_handle, &ev);
		}
	}

	end(wr_handle->writer, "events");

	return 0;
}