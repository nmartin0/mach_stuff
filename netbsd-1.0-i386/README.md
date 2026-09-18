# NetBSD 1.0 / i386

Mirrored verbatim from
https://archive.netbsd.org/pub/NetBSD-archive/NetBSD-1.0/i386/binary/
released 26 October 1994.

Copyright The NetBSD Foundation and contributors. NetBSD is
BSD-licensed and freely redistributable; the sets are unmodified.

Here because LITES has no userland of its own. Its own documentation,
doc/install.freebsd and doc/README.netbsd, says to install a BSD system
and add three files to /mach_servers. This is that BSD system.

The sets are split into 240640-byte pieces; cat them together for a
gzipped tar:

    cat base10/base10.* | tar xzf -
