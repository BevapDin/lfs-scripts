#include "FindFilesInMappedDirs.h"
#include "Package.h"
#include "TPKGS.h"
#include "RecursiveFileIterator.h"

#include <sys/stat.h>

FindFilesInMappedDirs::FindFilesInMappedDirs(const Package &package, const Version &version, const TPKGS &tp)
: FindFiles(package, version, tp)
{
}

FindFilesInMappedDirs::~FindFilesInMappedDirs() {
}

FindFiles::IIList &FindFilesInMappedDirs::loadItems() {
	FindFiles::clear();
	RecursiveFileIterator rfi(&std::cerr);

	const TPKGS::PathPairSet &pmap = tss.t.getInstallPathMap();
	for(TPKGS::PathPairSet::const_iterator a = pmap.begin(); a != pmap.end(); a++) {
		rfi.addPathToVisit(a->second.string());
	}

	rfi.run(*this);
	return result;
}

bool FindFilesInMappedDirs::operator()(const File::dir_iterator &a, const Path &path) {
	if(ignore(path)) {
		// Ignored paths are not considered at all, also its subdirs are ignored
		return false;
	}
	struct stat stat;
	if(::lstat(path.c_str(), &stat) != 0) {
		// stat failed, most likely permisson, broken link ... ignore it and any subdirs
		return false;
	}

	if(stat.st_gid == tss.p.getGroupID() || stat.st_uid == tss.p.getUserID()) {
		// File is owned by the package, add it to the list and scan its subdirs (if any)
		InstallItem ii = tss.fromInstallPath(path);
		result.push_back(ii);
	}

	// File is not owned by package, ignore it and any subfolders
	return false;
}

