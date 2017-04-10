#! /bin/bash

# This script shows files in /usr/src/*/ that need inspection.
# It works by gathering all files there and removing those that are
# either handled elsewhere (e.g. .url files are handled by
# gather-package-data.sh) or can be ignored safely (.bashrc)
# The listed files should *not* be there, they are usually:
# temporary files used during installation, e.g. the source archive,
# the unpacked source directory, or they are files that need
# manual interaction (e.g. artifacts of the installation process
# that can be removed).

cd /usr/src || exit ?$

find . -maxdepth 2 -mindepth 2 | \
sed '
	s#^\./##

	# expected for all packages:
	/\/\.url$/d
	/\/notes$/d
	/\/\.bash_profile$/d
	/\/\.bash_history$/d
	/\/\.bashrc$/d
	/\/\.viminfo$/d
	/\/\.lesshst$/d
	/\/.*\.log$/d

	/\/\..*.config$/d              # copy of a previously used myconfig file

	/\/\.subversion$/d
	/\/\.needs-pkg$/d
	/\/\.needed-by-pkg$/d
	/\/\.cache$/d
	/\/\.wget-hsts$/d

	/\/bin$/d                      # bin folders are to be expected
	/-ok.tar.bz2$/d                # -ok archives are fine as well

	# various checkouts:
	/^mplayer\/mplayer@svn/d
	/^boinc\/boinc/d

	/^lua\/lua\.pc$/d
	'
