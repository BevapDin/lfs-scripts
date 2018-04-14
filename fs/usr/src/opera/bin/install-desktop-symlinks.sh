#! /bin/bash

if [ -z "$1" ] ; then
	prefix="$(readlink /opt/opera)"
else
	prefix="$1"
fi

if [ -z "$prefix" ] ; then
	echo "Prefix not defined (/opt/opera)" 1>&2
	exit 1
fi

if ! [ -e "/opt/$prefix" ] ; then
	echo "Prefix invalid: /opt/$prefix" 1>&2
	exit 1
fi

echo "Using prefix = $prefix"

echo "Updating .desktop files:"
for file in "/opt/$prefix/share/applications/"* ; do
	if ! [ -e "$file" ] ; then
		echo "No files found!" 1>&2
		break
	fi
	echo "... $file"
	sed '
		s#@@{_SUFFIX}##g
		s#@@{SUFFIX}##g
		s#@@{PREFIX}#/opt/'"$prefix"'#g
	' -i "$file" || exit $?
	chmod 0755 "$file" || exit $?
done

echo "bin directory..."
mkdir -p /opt/$prefix/bin || exit $?

if ! [ -e "/opt/$prefix/bin/opera" ] ; then
	if [ -e "/opt/opera/bin/opera" ] ; then
		cp "/opt/opera/bin/opera" "/opt/$prefix/bin/opera"
	fi
fi

if false ; then
for file in opera opera-widget-manager ; do
	dest="/opt/$prefix/bin/$file"
	src="/opt/opera-11.52-1100.i386.linux/bin/$file"
	if ! [ -e "$src" ] ; then
		echo "Source not found: $src" 1>&2
		exit $?
	fi
	echo "Updating $dest"
	sed 's#opera-11.52-1100.i386.linux#'"$prefix"'#g' < "$src" > "$dest" && \
		chmod 0755 "$dest"
done
fi

if [ "$(readlink /opt/opera)" != "$prefix" ] ; then
	echo "Updating /opt/opera smylink"
	rm "/opt/opera" && ln -s "$prefix" "/opt/opera"
fi

