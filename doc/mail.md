# Mail

Install fetchmail, dovecot, postfix. No particular order needed.

## Installation of dovecot:

It tries to install the same files into "/usr/lib" and "/var/libexec", but I don't have "libexec" (I'm using "lib" instead). Therefor change that directory in the configuration:
```
--libexecdir=/usr/lib/dovecot/libexec
```
It can be any other directory, but remember it as it is needed later.

## fetchmail:

Install as usual. `fetchmail` can forward the fetched mails directly to dovecot via `deliver` (a program installed by dovecot). The program must be run as root, but fetchmail is run as local user, therefor enable `sudo` for it - add an entry in `sudoers`:
```
thomas ALL = (root) NOPASSWD:NOSETENV: /usr/lib/dovecot/libexec/dovecot/deliver
```
(Use the folder that was set up as "libexecdir" when installing dovecot plus "/dovecot/deliver" as final path.)

Set up fetchmailrc:
```
set no bouncemail

poll <your-mail-server> proto POP3
    user "Foo"
    pass "BAR"
    mda "/usr/bin/sudo -u root /usr/lib/dovecot/libexec/dovecot/deliver -d thomas -e -a thomas"
    no rewrite
    ;
```
