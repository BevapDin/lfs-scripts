# Updating postgresql

Well, that's going to be a bit more complicated.

Basic outline:
- start both servers (old one on a different post).
- create a FIFO.
- use `pg_dumpall` (from the old version) to dump the data into the FIFO.
- use `psql` (from the new version) to read the data from the FIFO.

Using a FIFO makes it faster as no temporary data gets written to the disk.

The LFS specific process is therefor this:

## start old server

This requires to chroot into the old system. Use the chroot command from the book (slightly adjusted) as root:
```BASH
chroot /mnt/old /usr/bin/env -i HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
	PATH=/bin:/usr/bin:/sbin:/usr/sbin /bin/bash --login
```

Edit the postgresql configuration (usually in /srv/pgsql/data) and change the `port` to something else (so it does not conflict with the new server).

Now start the old server (usually via `/etc/rc.d`):
```BASH
/etc/rc.d/init.d/postgresql start
```

## the FIFO

Find a place that is accessible from both systems (usually a point in the old system), e.g. `/mnt/old/tmp`. Create a FIFO there:
```BASH
mkfifo /mnt/old/tmp/pg-upgrade
```
The FIFO doesn't need special permissions, both programs (reader and writer) are started as root.

## dump data

In the old system, start `pg_dumpall`, redirect its output to the FIFO (use the path as it's seen from inside the chroot):
```BASH
pg_dumpall -U postgres -p 6000 -o -v >/tmp/pg-upgrade
```
`-o` dumps the oids as well, it may not be needed. `-v` is verbose and may not be needed as well.

## restore data

From the new system, run psql (as root):
```BASH
psql -U postgres </mnt/old/tmp/pg-update
```

Than you wait...


