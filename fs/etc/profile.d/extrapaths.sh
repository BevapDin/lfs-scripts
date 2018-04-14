
pathprepend /usr/lib/pkgconfig PKG_CONFIG_PATH

if [ -d /usr/local/lib/pkgconfig ] ; then
        pathappend /usr/local/lib/pkgconfig PKG_CONFIG_PATH
        pathappend /opt/lib/pkgconfig PKG_CONFIG_PATH
fi
if [ -d /usr/local/bin ]; then
        pathprepend /usr/local/bin
fi
if [ -d /usr/local/sbin -a $EUID -eq 0 ]; then
        pathprepend /usr/local/sbin
fi

pathappend /lib LD_LIBRARY_PATH
pathappend /usr/lib LD_LIBRARY_PATH

if [ -d ~/bin ]; then
        pathprepend ~/bin
fi
if [ -d ~/lib ] ; then
	pathprepend ~/lib LD_LIBRARY_PATH
fi

if [ -d /opt/lib ] ; then
	pathappend /opt/lib LD_LIBRARY_PATH
fi
if [ -d /opt/bin ] ; then
	pathappend /opt/bin
fi

#if [ $EUID -gt 99 ]; then
#        pathappend .
#fi
