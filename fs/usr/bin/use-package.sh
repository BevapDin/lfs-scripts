# This script must be sourced!

# This script sets some envirnoment varibales to include the
# pathes to the package specific folders in /package/<pkg>/<version>

# Usage: . $0 <pkg-name> <pkg-version>

pkg="${1?missing package}"
ver="${2?missing version}"

if [ ! -e "/packages/$pkg/$ver/" -o -z "$ver" ] ; then
	ls -1 "/packages/$pkg"
	return 1
fi

export PKG_CONFIG_PATH="$(
find "/packages/$pkg/$ver/" -type f -iname '*.pc' | \
	sed 's#/[^/]*$##;/^$/ d' | \
	sort | \
	uniq | \
	tr '\n' ':')":$PKG_CONFIG_PATH

export PATH="$(
find "/packages/$pkg/$ver/" -type d -name 'bin' -o -name 'sbin' | \
	tr '\n' ':')":$PATH

export LD_LIBRARY_PATH="$(
find "/packages/$pkg/$ver/" -type d -name 'lib' | \
	tr '\n' ':')":$LD_LIBRARY_PATH
