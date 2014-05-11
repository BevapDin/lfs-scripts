#include "FindFiles.h"
#include <errno.h>
#include "Package.h"
#include "Version.h"
#include "TPKGS.h"
#include "File.h"
#include "RecursiveFileIterator.h"
#include <sys/stat.h>
#include "FindFilesOnInstallDevice.h"
#include "FindFilesInInstallDirs.h"
#include "FindFilesInMappedDirs.h"


FindFiles::FindFiles(const Package &package, const Version &version, const TPKGS &tp)
: tss(tp, package, version)
, result()
, toIgnore()
{
}

FindFiles::~FindFiles() {
}

dev_t FindFiles::getDeviceID(const Path &path) {
	struct stat stat;
	if(::stat(path.c_str(), &stat) != 0) {
		throw SyscallException(errno, "stat", path);
	}
	return stat.st_dev;
}

FindFiles::IIList &FindFiles::getResult() {
	return result;
}

void FindFiles::addToIgnore(const Path &path) {
	toIgnore.insert(path);
}

void FindFiles::clear() {
	result.clear();
}

bool FindFiles::ignore(const Path &path) const {
	return toIgnore.find(path) != toIgnore.end();
}

void FindFiles::getNewFiles(PSA psAlgo, const TPKGS &tp, const Package &p, const Version &v, IIList &iilist) {
	switch(psAlgo) {
		case PSA_INSTALL_DEVICE:
			{
				FindFilesOnInstallDevice ffoid(p, v, tp);
				iilist.swap(ffoid.loadItems());
			}
			break;
		case PSA_MAPPED_DIRS:
			{
				FindFilesInMappedDirs ffoid(p, v, tp);
				iilist.swap(ffoid.loadItems());
			}
			break;
		case PSA_INSTALL_DIRS:
			{
				FindFilesInInstallDirs ffoid(p, v, tp);
				iilist.swap(ffoid.loadItems("/etc/install-dirs"));
			}
			break;
		default:
			assert(false);
			throw std::runtime_error("invalid package source algorithm");
	}
	if(iilist.empty()) {
		throw std::runtime_error("no files for the package found");
	}
}

