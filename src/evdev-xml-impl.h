/*
 * Copyright (C) 2018 Kynetics, LLC
 *
 * Copyright (C) 2010-2012 Canonical Ltd.
 * Copyright (C) 2010 Henrik Rydberg <rydberg@euromail.se>
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
 */
#ifndef EVDEV_XML_IMPL_H
#define EVDEV_XML_IMPL_H

#include <evdev-xml.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <libxml/xmlwriter.h>

struct evdev_device {
	struct libevdev *evdev;
	struct libevdev_uinput *uidev;
	/* we read in properties and bits 8 at a time, but the file format
	 * has no hint which byte we're up to. So we count what we've read
	 * already to know where the next one tacks onto */
	int pbytes, mbytes[EV_CNT];
};

struct xml_writer {
	xmlTextWriterPtr writer;
};

#endif
