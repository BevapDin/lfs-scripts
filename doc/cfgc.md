cfgc:
=====
This script resides in /usr/bin. It started out as a simple wrapper around `./configure && make && make install`.

Than I added a wrapper that stored the result of `./configure --help` and formated that a bit to make it easy to add/remove/change options:
```
#--with-avahi=DIR
# disable support for jpeg
#--without-jpeg
# use jpeg include/library files in DIR
#--with-jpeg=DIR
# disable support for deflate
#--without-libz
# disable support for deflate
```
Adding the `--without-jpeg` is as simple as removing the `#` in front of it.
This formated help also applied some automatic values like `--sysconfdir /etc` when it sensed that option.
It was orignally inted as a simple bash script (therefor the `#` as comment).
Now it's a bit more complicated (`cfgc` create a temporary script starting with `./configure` and than the content of `myconfig` where everything from `#` to the line end had been removed).

Than I added log files (`{config,make,install}.{1,2}.log` in the users home dir).

Than I added wrapper for some other configure/build systems like cmake.

And it's still growing.
