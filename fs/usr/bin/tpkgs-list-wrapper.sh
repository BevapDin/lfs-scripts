#! /bin/bash

TPKGS=tpkgs

items=$($TPKGS --list "$@" | sed '/^\[.\] / !d; /^\[.\] \.\.\. / d; s#^\[##;s#\].*$##' | sort | uniq)

for d in $items ; do
	case $d in
		e)
			echo "Links / folders exist"
			;;
		c)
			echo "Links / folders will be created"
			;;
		i)
			echo "Links / folders will be ignored by config"
			;;
		m)
			echo "WARNING: unmapped things:"
			$TPKGS --list "$@" | sed '/^\['$d'\] / !d'
			;;
		\?)
			echo "WARNING: uninstallable files"
			$TPKGS --list "$@" | sed '/^\['$d'\] / !d'
			;;
		*)
			echo "Unknown $d"
			$TPKGS --list "$@" | sed '/^\['$d'\] / !d'
			;;
	esac
done


