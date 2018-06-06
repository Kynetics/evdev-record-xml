evemu - Kernel device emulation
===============================

The evemu library and tools are used to describe devices, record
data, create devices and replay data from kernel evdev devices.

* http://www.freedesktop.org/wiki/Evemu
* https://gitlab.freedesktop.org/libevdev/evtest

Development
-----------

Discussions and development of evemu happens on the mailing list

     input-tools@lists.freedesktop.org

<http://lists.freedesktop.org/mailman/listinfo/input-tools>

Please submit signed-off git-formatted patches with a subject prefix
of "PATCH evemu" to this list. The prefix can be set up by running

     git config --add format.subjectprefix "PATCH evemu"

in the git repository.

Bugs
----

Please report bugs in the bugzilla component at freedesktop.org

<https://gitlab.freedesktop.org/libevdev/evtest/issues>


Device Description Format
-------------------------

evemu produces two different data formats, one for the device description
and one for the device event data. hex data is without a 0x prefix.

    # EVEMU 1.3
    # comments start with a # character and go to the end of the line
    N: <device name>
    I: <bustype (hex)> <vendor (hex)> <product (hex)> <version (hex)>
     --- for each kernel property (2.3.38 only) ---
    P: <byte 0 (hex)> <byte 1 (hex)> ... <byte 7 (hex)>
    P: <byte 8 (hex)> ...
     --- for each index from 0 to EV_CNT ---
    B: <index (hex)> <byte 0 (hex)> <byte 1 (hex)> ... <byte 7 (hex)>
    B: <index (hex)> <byte 8 (hex)> ...
     --- for each absolute axis ---
    A: <index (hex)> <min> <max> <fuzz> <flat> <resolution>
     --- for each LED ---
    L: <index (hex)> <state>
     --- for each switch ---
    S: <index (hex)> <state>

The first line is a special comment and taken to describe the file format
version. It is always comment character (#), space, "EVEMU", space, followed
by a numeric version number in the format major.minor.
If the line is missing, file format 1.0 is assumed.

If L and S are missing, the state of each LED/switch is 0

minor version numbers are additions of new fields, or alterations of a
field.
major version numbers are large redesigns of the format

Current file format versions supported:
 * 1.0 - comments may only be present at the top of the file, before any
	 data
 * 1.1 - comments may be present at any line of the file, including at the
	 end of a line
 * 1.2 - abs axis information has the resolution appended
 * 1.3 - LED/switch state added

Event Data Format
-----------------

    E: <sec>.<usec> <evtype (hex)> <evcode (hex)> <ev value>
where type, code and value are the respective fields of the
input_event struct defined in linux/input.h

Comments
--------

evemu description files may include comments, prefixed by the # character.
Comments may appear on their own in an otherwise empty line, or may be
appended to lines holding data. For example, both of these comments are
valid:

    # next line is for ABS_FOO
    A: 1 3 4 5 6 7 # ABS_FOO

Comments are not recognized in the device name line, as a # may be part
of the device's name. Thus

    N: foo # bar

describes a device with the name "foo # bar"


Copyright
---------

 * Copyright (C) 2010 Henrik Rydberg <rydberg@euromail.se>
 * Copyright (C) 2010 Canonical Ltd.
 * Copyright (C) 2013-2015 Red Hat, Inc.
