/*****************************************************************************
 *
 * evemu - Kernel device emulation
 *
 * Copyright (C) 2018 Kynetics, LLC
 *
 * Copyright (C) 2010-2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "evdev-xml.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "find_event_devices.h"

#define INFINITE -1

static char *outfile = "evdev-record.xml";

struct xml_writer *wr_handle;

static int describe_device(int fd)
{
    struct evdev_device *dev;
    int ret = -ENOMEM;

    dev = evdev_dev_new(NULL);
    if (!dev)
        goto out;
    ret = evdev_dev_extract(dev, fd);
    if (ret)
        goto out;

    evdev_xml_print_dev_info(wr_handle, dev);
out:
    evdev_dev_delete(dev);
    return ret;
}

static void handler(int sig __attribute__((unused)))
{
    fflush(stdout);
}

static inline void usage()
{
    fprintf(stderr, "Usage: evdev-record-xml <device> [output file]\n");
}

static void record_device(int fd, unsigned int timeout)
{
    wr_handle = evdev_xml_writer_new();
    if (!wr_handle)
    {
        fprintf(stderr, "error: could not initialize xml document.\n");
        return;
    }

    evdev_xml_init(wr_handle, outfile);

    if (describe_device(fd))
    {
        fprintf(stderr, "error: could not describe device\n");
        return;
    }

    fprintf(stdout, "################ Recording ################\n");
    fprintf(stdout, "Saving data to: %s\n", outfile);
    fprintf(stdout, "Press ^C to interrupt recording.\n");

    if (evdev_xml_record_dev(wr_handle, fd, timeout))
    {
        fprintf(stderr, "error: could not record device\n");
    }

    evdev_xml_close(wr_handle);
}

static inline bool test_grab_device(int fd)
{
    if (ioctl(fd, EVIOCGRAB, (void *)1) < 0)
    {
        fprintf(stderr, "error: this device is grabbed and I cannot record events\n");
        return false;
    }
    else
    {
        ioctl(fd, EVIOCGRAB, (void *)0);
    }

    return true;
}

int main(int argc, char *argv[])
{
    int fd = -1;
    struct sigaction act;
    char *device = NULL;
    int timeout = INFINITE;

    int rc = 1;

    while (1)
    {
        int c;

        c = getopt (argc, argv, "");

        if (c == -1)
            break;

        usage();
        goto out;
    }

    /* Get input device */
    device = (optind >= argc) ? find_event_devices() : strdup(argv[optind++]);

    if (device == NULL)
    {
        usage();
        goto out;
    }
    fd = open(device, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        fprintf(stderr, "error: could not open device (%m)\n");
        goto out;
    }

    /* Get output file name */
    if (optind < argc)
        outfile = strdup(argv[optind++]);

    memset(&act, '\0', sizeof(act));
    act.sa_handler = &handler;

    if (sigaction(SIGTERM, &act, NULL) < 0)
    {
        fprintf(stderr, "Could not attach TERM signal handler (%m)\n");
        goto out;
    }
    if (sigaction(SIGINT, &act, NULL) < 0)
    {
        fprintf(stderr, "Could not attach INT signal handler (%m)\n");
        goto out;
    }

#ifdef EVIOCSCLOCKID
    int clockid = CLOCK_REALTIME;
    ioctl(fd, EVIOCSCLOCKID, &clockid);
#endif
    if (!test_grab_device(fd))
        goto out;

    record_device(fd, timeout);
    rc = 0;
out:
    free(device);
    close(fd);
    return rc;
}
