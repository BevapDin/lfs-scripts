#include "Package.h"
#include <errno.h>
#include <stdexcept>
#include <cassert>
#include <stdlib.h>
#include <sys/stat.h>
#include "File.h"
#include "InstallItem.h"
#include "TPKGS.h"
#include <boost/filesystem/operations.hpp>

#include <cstdlib>

#define CONFIG_FILE_FILENAME ".tpkgs2.conf"

static const std::string cvl("Current");
static const Package::UID INVALID_UID(static_cast<Package::UID>(-1));
static const Package::GID INVALID_GID(static_cast<Package::GID>(-1));

extern std::string trim(const std::string &text);

Package::NoSuchVersionException::NoSuchVersionException(const Package &p, const Version &v)
: std::runtime_error(std::string("package ") + p.getName() + " has no version " + v.getName())
{
}

Package::Package(const Name &name)
: _name(trim(name))
, _packageDir()
, _existingVersions()
, _installedVersion()
, _userID(INVALID_UID)
, _groupID(INVALID_GID)
{
}

Package::~Package() {
}

bool Package::isInstalled() const {
	return !_installedVersion.isEmptyVersion();
}

bool Package::isInstalled(const Version &version) const {
	return version == _installedVersion;
}

bool Package::hasVersions() const {
	return !_existingVersions.empty();
}

bool Package::hasVersion(const Version &version) const {
	return _existingVersions.count(version) > 0;
}

Version Package::getInstalledOrRecentVersion() const {
	if(isInstalled()) {
		return getInstalledVersion();
	} else if(hasVersions()) {
		return getRecentVersion();
	} else {
		return Version::EMPTY_VERSION;
	}
}

Version Package::getRecentVersion() const {
	if(_existingVersions.empty()) {
		return Version::EMPTY_VERSION;
	}
	return *(_existingVersions.rbegin());
}

Version Package::getSecondRecentVersion() const {
	if(_existingVersions.size() < 2) {
		return Version::EMPTY_VERSION;
	}
	auto a = _existingVersions.rbegin();
	a++;
	return *a;
}

Package::Path Package::getVersionPath(const Version &version) const {
	return _packageDir / version.getName();
}

void Package::gatherInstallItemList(const Version &version, const TPKGS &tpkgs, IIList &list) const {
	checkVersionExists(version);
	std::set<Path> dirsToVisit;
	dirsToVisit.insert(getVersionPath(version));
	InstallItem::TSS tss(tpkgs, *this, version);
	while(!dirsToVisit.empty()) {
		File d(*(dirsToVisit.begin()));
		dirsToVisit.erase(dirsToVisit.begin());
		for(File::dir_iterator a = d.begin(); a != d.end(); ++a) {
			const std::string path = a.getPath() + a.getFile();
			if(!a.isLink() && a.isDir()) {
				dirsToVisit.insert(path);
			}
			InstallItem ii = tss.fromSourcePath(path);
			list.push_back(ii);
		}
	}
}

bool isRunningAsRoot() {
	static uid_t uid = getuid();
	return (uid == 0);
}

void Package::initIDs() {
	if(_userID != INVALID_UID && _groupID != INVALID_GID) {
		return;
	}
	struct stat st;
	if(::lstat(_packageDir.string().c_str(), &st) != 0) {
		// ^^ Failed, try home dir
		const int e = errno; // save for later meaningfull exception
		const char *home = std::getenv("HOME");
		if(!isRunningAsRoot() && home != NULL && ::lstat(home, &st) == 0) {
			boost::filesystem::create_directory(_packageDir);
		} else { // running as root || home == NULL || lstat failed
			throw SyscallException(e, "stat", _packageDir.string());
		}
	}
	_userID = st.st_uid;
	_groupID = st.st_gid;
}

void Package::init(const TPKGS &tpkgs) {
	if(_name.find_first_not_of("/\\ \n\t") == std::string::npos) {
		// _name contains only (path separators or spaces)
		throw std::runtime_error("invalid package _name: empty");
	}
	_packageDir = tpkgs.getMainPackageDir() / _name;
	initIDs();
	initVersions();
}

void Package::initVersions() {
	_existingVersions.clear();
	_installedVersion = Version::EMPTY_VERSION;

	File pp(_packageDir);
	for(File::dir_iterator a = pp.begin(); a != pp.end(); ++a) {
		if(a.isHidden()) {
			continue;
		}
		if(a.isLink()) {
			if(a.getFile() == cvl) {
				const std::string t = a.readlink();
				if(t.find_first_of("/\\") != std::string::npos) {
					throw std::runtime_error(_name + ": invalid current symlink (contains '/'): " + t);
				}
				if(!File::isDir((_packageDir / t).string())) {
					throw std::runtime_error(_name + ": current version is invalid (not a dir): " + t);
				}
				assert(_installedVersion.isEmptyVersion());
				_installedVersion = Version(t);
			}
			continue;
		}
		if(!a.isDir()) {
			continue;
		}
		_existingVersions.insert(Version(a.getFile()));
	}
}

bool Package::isOwned(const File &file) const {
	struct stat st;
	if(::lstat(file.asString().c_str(), &st) != 0) {
		throw SyscallException(errno, "stat", file.asString());
	}
	return (st.st_uid == _userID && st.st_gid == _groupID);
}

void Package::setOwner(const File &file) const {
	if(isOwned(file)) {
		return;
	}
	if(::lchown(file.asString().c_str(), _userID, _groupID) != 0) {
		throw SyscallException(errno, "lchown", file.asString());
	}
}

void Package::setInstalledVersion(const Version &version) {
	setUninstalled();
	const File cur(getVersionPath(cvl));
	assert(!cur.exists());
	cur.link(version.getName());
}

void Package::setUninstalled() {
	const File cur(getVersionPath(cvl));
	if(cur.exists()) {
		if(!cur.isLink()) {
			throw std::runtime_error(_name + ": current symlink is not a symlink");
		}
		cur.unlink();
	}
}

Package::Path Package::getConfigFile() const {
	return _packageDir / CONFIG_FILE_FILENAME;
}

Package::Path Package::getConfigFile(const Version &version) const {
	return _packageDir / version.getName() / CONFIG_FILE_FILENAME;
}

void Package::checkVersionExists(const Version &version) const {
	if(!hasVersion(version)) {
		throw NoSuchVersionException(*this, version);
	}
	if(version.isEmptyVersion()) {
		throw NoSuchVersionException(*this, Version("<the empty version string>"));
	}
}
