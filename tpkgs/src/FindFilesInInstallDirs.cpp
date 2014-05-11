#include "FindFilesInInstallDirs.h"
#include "Package.h"
#include "TPKGS.h"

#include <sys/stat.h>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <fstream>

#include "RecursiveFileIterator.h"

FindFilesInInstallDirs::FindFilesInInstallDirs(const Package &package, const Version &version, const TPKGS &tp)
: FindFiles(package, version, tp)
{
}

FindFilesInInstallDirs::~FindFilesInInstallDirs() {
}

FindFiles::IIList &FindFilesInInstallDirs::loadItems(const Path &installDirsFile) {
	FindFiles::clear();
	RecursiveFileIterator rfi(&std::cerr);

	std::ifstream installFolderFileStream;
	installFolderFileStream.exceptions(std::ios::badbit);
	installFolderFileStream.open(installDirsFile.c_str(), std::ios::in);
	std::string line;
	while(!!getline(installFolderFileStream, line)) {
		boost::trim(line);
		if(!line.empty()) {
			rfi.addPathToVisit(line);
		}
	}

	rfi.run(*this);
	return result;
}

bool FindFilesInInstallDirs::operator()(const File::dir_iterator &a, const Path &path) {
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
		return !ii.is_ignore();
	}

	// File is not owned by package, ignore it and any subfolders
	return false;
}

