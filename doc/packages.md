About installing packages:

First you want to decide on the name of the package user. For many packages this can be the package name (I usually use the name of the downloaded source archive, minus the version). However sometimes package names don't fit the name restrictions supposed by system and one has to be creative.

Call `install_package` with the package name as parameter to create the package user and group and instantly `su` into that users account. The script requires root permissions as it creates new users. I added a rule to "/etc/sudoers" which allows me to call this script directly from the user account I usually work with
Running it looks like this:
```
sudo /usr/sbin/install_package codeblocks
```

Once inside the package user, one has to get the source. I usually put the source archive directly into the "$HOME" and extract it there as well. Later, after the installation has finished, I move it into a backup directory.

There is the `new` script (only available for package users), it looks for a "$HOME/.url" file and (if it's there) starts lynx with the URL taken from that file. If the file does not exists it starts lynx with a goggle search for the current user name. Usually I browse the internet until I found a worthy package, then call `install_package`, then either write the URL of the download package (if I already know it) into the ".url" file and run `new`. If the download page is not yet known, I just run `new`, go through the search results and locate the download page. Anyway, the source gets downloaded, if required the ".url" is updated (upon the next invocation it will automatically point to the correct site).

Next step is extracting the archive, and running `cfgc` (if there is a configure script), or do other package specific installation routines. Note that `cfgc` will upon the first invocation in a source folder (the myconfig file does not exist) show the Readme or Install file (whatever exists).


