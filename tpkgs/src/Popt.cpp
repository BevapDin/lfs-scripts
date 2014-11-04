#include "Popt.h"

#include "TPKGS.h"
#include "Package.h"
#include "Version.h"


#include "File.h"
#include "MXOSet.h"
#include <stdlib.h>

#include <iostream>
#include <string.h>
#include <stdexcept>
#include <vector>
#include <cassert>

#include "FindFilesOnInstallDevice.h"
#include "FindFilesInMappedDirs.h"
#include "FindFilesInInstallDirs.h"

Popt::Popt(TPKGS &tp)
: _optCon(NULL)
, _args()
, _tp(tp)
{
}

Popt::~Popt() {
	if(_optCon != NULL) {
		poptFreeContext(_optCon);
	}
}

std::string Popt::popArg() {
	return popArg("missing argument(s)");
}

size_t Popt::getArgCount() const {
	return _args.size();
}

std::string Popt::popArg(const std::string &errorString) {
	if(_args.empty()) {
		throw MissingArgumentException(errorString);
	}
	const std::string result = _args.front();
	_args.erase(_args.begin());
	return result;
}

void Popt::init(int argc, char *argv[], struct poptOption *optionsTable, const char *oh) {
	if(_optCon != NULL) {
		poptFreeContext(_optCon);
	}
	_optCon = poptGetContext(NULL, argc, const_cast<const char**>(argv), optionsTable, 0);
	assert(_optCon != NULL);
	poptSetOtherOptionHelp(_optCon, oh);
	const int opt = poptGetNextOpt(_optCon);
	if(opt != -1) {
		throw std::runtime_error("invalid command line");
	}
	_args.clear();
	const char *arg;
	while((arg = poptGetArg(_optCon)) != NULL) {
		_args.push_back(arg);
	}
}

void Popt::usage() {
	assert(_optCon != NULL);
	poptPrintUsage(_optCon, stderr, 1);
}

void Popt::usage(const std::string &message) {
	std::cerr << message << "\n";
	usage();
}

void Popt::removePrefix(std::string &s, const std::string &prefix) {
	if(s.length() > prefix.length() && s.compare(0, prefix.length(), prefix) == 0) {
		s.erase(0, prefix.length());
	}
}

Package &Popt::getPackage() {
	Version dummy;
	return getPackage(dummy, NONE_NEEDED, false);
}

Package &Popt::getPackage(Version &v1, Version &v2) {
	v1 = v2 = Version::EMPTY_VERSION;
	Package &p = getPackage(v1, NONE_NEEDED, false);
	if(!hasMoreArgs()) {
		// No more args, v1 may be set
		return p;
	}
	if(v1.isEmptyVersion()) { // v1 is empty and we have more arguments, extract v1 from them
		v1 = getVersion(p, NO_DEFAULT, false);
		assert(!v1.isEmptyVersion());
		if(!hasMoreArgs()) {
			// No more args, v1 is set, v2 not
			return p;
		}
	}
	assert(hasMoreArgs()); // We did check this after the above getPackage and getVersion calls.
	assert(v2.isEmptyVersion()); // Never written to it
	v2 = getVersion(p, NO_DEFAULT, false);
	assert(!v2.isEmptyVersion());
	return p;
}

Package &Popt::getPackage2(Version &v) {
	if(_args.size() == 1) {
		// Only one argument, assume this to be the version
		std::string pname = popArg();
		if(extractPackageAndVersionFromString(pname, v)) {
			// pname had a package name _and_ a version, that's all we need, v is already written.
			return _tp.getPackage(pname);
		}
		// Now we have either a package name and no version string
		// or only a version string and no package name.
		// See if we have a default package name.
		const std::string defaultPName = getDefaultPackageName();
		if(!defaultPName.empty()) {
			// OK, we have a default package name, assume the argument to be the version
			v = pname;
			return _tp.getPackage(defaultPName);
		}
		// Initialy there was only one argument, it had been removed at the call to popArg
		// above, now the getPackage will fail with a more or less meaningfull message.
		assert(_args.empty());
	}
	// At least two arguments, use the normal algorithm
	return getPackage(v, NO_DEFAULT, false);
}

bool Popt::extractPackageAndVersionFromString(std::string &pname, Version &v) const {
	removePrefix(pname, _tp.getMainPackageDir().string());
	removePrefix(pname, "/usr/src/");
	removePrefix(pname, "/");
	const size_t i = pname.find(':');
	if(i == std::string::npos) {
		return false;
	}
	// Package string contains the version: <pkg>:<version>
	v = Version(pname.substr(i + 1));
	pname.erase(i, pname.length() - i);
	return true;
}

std::string Popt::getDefaultPackageName() {
	// Assume that we are currently running as the install user.
	// Only if the current user id is 0 (root) we need the package name on the command line.
	if(getuid() != 0) { // TODO: there is a is-root-function somewhere!
		const char *p = std::getenv("LOGNAME");
		if(p != nullptr) {
			return std::string(p);
		}
	}
	return std::string();
}

void Popt::warnIfPackageExists(const std::string &pname, const std::string &oname) const {
	if(!_tp.hasPackage(oname)) {
		return;
	}
	std::cerr << "warning: package " << pname << " does not (yet) exist, ";
	std::cerr << "did you mean " << oname << "?" << std::endl;
}

// Extract package and version, both may be in on argument as
// <pkg>:<version> or they may be in two arguments.
// @param versionMustExist If true, the function checks that the version supplied
// actually exists, otherwise it throws an exception.
Package &Popt::getPackage(Version &v, VersionFlags vf, bool versionMustExist) {
	if(_args.empty()) {
		const std::string defaultPName = getDefaultPackageName();
		if(!defaultPName.empty()) {
			Package &p = _tp.getPackage(defaultPName);
			if(vf != NONE_NEEDED) {
				v = getVersion(p, vf, versionMustExist); // extract with default or throw
			}
			return p;
		}
	}
	std::string pname = popArg("missing package name");
	if(extractPackageAndVersionFromString(pname, v)) {
		Package &p = _tp.getPackage(pname);
		if(versionMustExist) {
			p.checkVersionExists(v);
		}
		return p;
	}
	if(!_tp.hasPackage(pname)) {
		if(pname.compare(0, 3, "lib") == 0) {
			// package name is something like "libfoo"
			// Check if the package name is actually only "foo" or "foolib"
			Package::Name oname = pname.substr(3);
			warnIfPackageExists(pname, oname);
			warnIfPackageExists(pname, oname + "lib");
		} else if(pname.length() > 3 && pname.compare(pname.length() - 3, 3, "lib") == 0) {
			// Same as above, but package name is "foolib"
			Package::Name oname = pname.substr(0, pname.length() - 3);
			warnIfPackageExists(pname, oname);
			warnIfPackageExists(pname, "lib" + oname);
		} else {
			warnIfPackageExists(pname, pname + "lib");
			warnIfPackageExists(pname, "lib" + pname);
		}
		const auto defaultPName = getDefaultPackageName();
		if(!defaultPName.empty() && vf != NONE_NEEDED && _tp.hasPackage(defaultPName)) {
			// Command line had no existing package, maybe it's the version number only?
			Package &p = _tp.getPackage(defaultPName);
			_args.insert(_args.begin(), pname);
			v = getVersion(p, vf, versionMustExist);
			return p;
		}
	}
	Package &p = _tp.getPackage(pname);
	if(vf != NONE_NEEDED) {
		v = getVersion(p, vf, versionMustExist);
	}
	return p;
}

bool Popt::hasMoreArgs() const {
	return !_args.empty();
}

Version Popt::getVersion(const Package &p, VersionFlag vf, bool versionMustExist) {
	if(_args.empty()) {
		switch(vf) {
			case NO_DEFAULT:
				// No default, skip to the popArg call below which
				// will trigger the appropriate error.
				break;
			case INSTALLED:
				if(p.isInstalled()) {
					return p.getInstalledVersion(); // exists per definition
				}
				break; // Fall through to popArg
			case INSTALLED_OR_CURRENT:
				if(p.isInstalled()) {
					return p.getInstalledVersion(); // exists per definition
				}
				// fall through to next case
			case CURRENT:
				if(p.hasVersions()) {
					return p.getRecentVersion(); // Exists per definition
				}
				break; // Fall through to popArg
			case NONE_NEEDED:
				// Ignores versionMustExist, caller has to handle this,
				// but at least we do not return an invalid version just an empty
				// one, the caller can easily distinct them.
				return Version::EMPTY_VERSION;
		}
	}
	const Version v(popArg("missing version"));
	if(v.isEmptyVersion() || versionMustExist) {
		p.checkVersionExists(v);
	}
	return v;
}

void Popt::checkNoMoreArgs() const {
	if(!_args.empty()) {
		throw std::runtime_error("there are more parameters on the command line than needed");
	}
}
