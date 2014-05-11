#include "Version.h"

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include <string.h>

const Version Version::EMPTY_VERSION;

Version::Version()
: _name()
{
}

Version::Version(const Name &name)
: _name(name)
{
}

Version::~Version() {
}

bool Version::operator<(const Version &other) const {
	if(_name.empty() != other._name.empty()) {
		return _name.empty();
	}
	return (strverscmp(_name.c_str(), other._name.c_str()) < 0);
}


