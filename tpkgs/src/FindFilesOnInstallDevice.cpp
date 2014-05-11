#include "FindFilesOnInstallDevice.h"
#include <errno.h>
#include "Package.h"
#include "Version.h"
#include "TPKGS.h"
#include "File.h"
#include "RecursiveFileIterator.h"
#include <sys/stat.h>

FindFilesOnInstallDevice::FindFilesOnInstallDevice(const Package &package, const Version &version, const TPKGS &tp)
: FindFiles(package, version, tp)
, mainInstallPathDeviceID(-1)
{
}

FindFilesOnInstallDevice::~FindFilesOnInstallDevice() {
}

FindFilesOnInstallDevice::IIList &FindFilesOnInstallDevice::loadItems() {
	FindFiles::clear();
	RecursiveFileIterator rfi(&std::cerr);
	mainInstallPathDeviceID = getDeviceID("/");
	rfi.addPathToVisit("/");
	rfi.run(*this);
	return result;
}

bool FindFilesOnInstallDevice::operator()(const File::dir_iterator &a, const Path &path) {
	if(ignore(path)) {
		// Ignored paths are not considered at all, also its subdirs are ignored
		return false;
	}
	struct stat stat;
	if(::lstat(path.c_str(), &stat) != 0) {
		// stat failed, most likely permisson, broken link ... ignore it and any subdirs
		return false;
	} else if(stat.st_dev != mainInstallPathDeviceID) {
		// file is not on the root device, ignore it and any subfolders
		return false;
	}

	if(stat.st_gid == tss.p.getGroupID() || stat.st_uid == tss.p.getUserID()) {
		// File is owned by the package, add it to the list
		InstallItem ii = tss.fromInstallPath(path);
		result.push_back(ii);
	}

	// Scan any subfolders allways.
	return true;
}
