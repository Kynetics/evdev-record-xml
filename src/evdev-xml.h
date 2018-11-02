/*****************************************************************************
 *
 * evemu - Kernel device emulation
 *
 * Copyright (C) 2018 Kynetics, LLC
 *
 * Copyright (C) 2010-2012 Canonical Ltd.
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

#ifndef EVDEV_XML_H
#define EVDEV_XML_H

#include <stdio.h>
#include <errno.h>
#include <linux/input.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define EVEMU_VERSION 0x00010000

extern struct xml_writer;

/**
 * evdev_dev_new() - allocate a new evdev device
 * @name: wanted input device name (or NULL to leave empty)
 *
 * This function allocates a new evdev device structure and
 * initializes all fields to zero. If name is non-null and the length
 * is sane, it is copied to the device name.
 *
 * Returns NULL in case of memory failure.
 */
struct evdev_device *evdev_dev_new(const char *name);

/**
 * evdev_dev_delete() - free and allocated evdev device
 * @dev: the device to free
 *
 * The device pointer is invalidated by this call.
 */
void evdev_dev_delete(struct evdev_device *dev);

/**
 * evdev_dev_set_name() - set device name
 * @dev: the device in use
 *
 * Sets the name of the device. If name is non-null and the length is
 * sane, it is copied to the device name.
 */
void evdev_dev_set_name(struct evdev_device *dev, const char *name);

/**
 * evdev_dev_extract() - configure evdev instance directly from the kernel device
 * @dev: the device in use
 * @fd: file descriptor of the kernel device to query
 *
 * Returns zero if successful, negative error otherwise.
 */
int evdev_dev_extract(struct evdev_device *dev, int fd);

/**
 * evdev_dev_destroy() - destroy all created kernel devices
 * @dev: the device to destroy
 *
 * Destroys all devices created using this file descriptor.
 */
void evdev_dev_destroy(struct evdev_device *dev);

/**
 * evdev_xml_writer_new() - allocate a new xml writer
 *
 * This function allocates a new xml_writer struct object and
 * initializes all fields to zero.
 *
 * Returns NULL in case of memory failure.
 */
struct xml_writer *evdev_xml_writer_new();

/**
 * evdev_xml_init() - initialize new xml document
 * @wr_handle: xml writer object
 * @outfile: wanted name of the output file
 *
 * This function initializes a new xml document with the specified
 * name.
 *
 * Returns -1 in case of failure.
 */
int evdev_xml_init(struct xml_writer *wr_handle, const char *outfile);

/**
 * evdev_xml_close() - finalize previously opened xml document
 * @wr_handle: xml writer object
 *
 * This function finalizes and closes a previously opened xml
 * document.
 *
 * Returns -1 if no xml document was previously opened.
 */
int evdev_xml_close(struct xml_writer *wr_handle);

/**
 * evdev_xml_print_dev_info () - print input device info
 * @wr_handle: xml writer object
 * @dev: the input device
 *
 * This function prints information about the specified input device
 * to the xml document.
 *
 * Returns -1 in case of failure.
 */
int evdev_xml_print_dev_info(struct xml_writer *wr_handle, const struct evdev_device *dev);

/**
 * evdev_xml_record_dev() - record input device events
 * @wr_handle: xml writer object
 * @fd: file descriptor of the input device to be polled
 * @ms: poll timeout in ms
 *
 * This function records the events of the specified input
 * device and save the information to the xml document.
 *
 * Returns -1 in case of failure.
 */
int evdev_xml_record_dev(struct xml_writer *wr_handle, int fd, int ms);

#ifdef __cplusplus
}
#endif

#endif
