#include "InstallItem.h"
#include <cassert>
#include "MXOSet.h"
#include <stdexcept>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "File.h"
#include "Package.h"
#include "TPKGS.h"
#include "MMap.h"
#include <memory.h>
#include <stdio.h>
#include <boost/filesystem/operations.hpp>
namespace fs = boost::filesystem;

InstallItem::InstallItem()
: _relSourcePath()
, _absSourcePath()
, _absInstalledPath()
, _type(T_OTHER)
, _installContent()
, _keep(false)
, _ignore(false)
, _mapped(false)
{
}

InstallItem::~InstallItem() {
}

InstallItem::Type InstallItem::getType(const File &file) {
	if(file.isLink()) {
		return T_LINK;
	} else if(file.isDir()) {
		return T_DIR;
	} else if(file.isReg()) {
		return T_REG;
	} else if(file.exists()) {
		return T_OTHER;
	} else {
		return T_NON_EXIST;
	}
}

void InstallItem::installLink(MXOSet &mxo) const {
	if(fs::is_symlink(_absInstalledPath)) {
		// _absInstalledPath is a link => relink or unchanged or error?
		const Path dstLink = fs::read_symlink(_absInstalledPath);
		if(dstLink == _installContent) {
			// unchanged, is already fine
			mxo.addNote('-', "already exist with correct content", _absSourcePath, _absInstalledPath, _installContent);
			mxo.noteStrangeOwner(_absInstalledPath);
		} else if(!mxo.overwriteWrongLinks()) {
			// error (not allowed to change the link)
			mxo.addError('F', "exists with wrong content", _absSourcePath, _absInstalledPath, dstLink);
		} else if(!mxo.isOwned(_absInstalledPath) && !mxo.overwriteForeignLinks()) {
			// error (other owner, not allowed to change those link)
			mxo.addError('P', "wrong link _installContent, not owned by package user", _absSourcePath, _absInstalledPath, dstLink);
		} else {
			// relink
			mxo.addNote('A', "wrong link _installContent, will be relinked", _absSourcePath, _absInstalledPath, dstLink);
			if(!mxo.dryRun()) {
				fs::remove(_absInstalledPath);
				fs::create_symlink(_installContent, _absInstalledPath);
				mxo.setOwner(_absInstalledPath);
			}
		}
	} else if(fs::exists(_absInstalledPath)) {
		// _absInstalledPath is something else (no link) => error!
		mxo.addError('L', "destination exists, is no link", _absSourcePath, _absInstalledPath);
	} else {
		// _absInstalledPath does not exists => simply create
		mxo.addNote('+', "destination will be created", _absSourcePath, _absInstalledPath, _installContent);
		if(!mxo.dryRun()) {
			fs::create_symlink(_installContent, _absInstalledPath);
			mxo.setOwner(_absInstalledPath);
		}
	}
}

bool areFilesEqual(const fs::path &a, const fs::path &b) {
	MMap ma(a.string());
	MMap mb(b.string());
	if(ma.len() != mb.len()) {
		return false;
	}
	return std::memcmp(ma.map(), mb.map(), ma.len()) == 0;
}

// Move or copy the file, reutnr weather it has been moved ot copied
void moveOrCopy(const fs::path &source, const fs::path &destination) {
	// First try a simple rename.
	boost::system::error_code ec;
	fs::rename(source, destination, ec);
	if(!ec) {
		return;
	}
#if 0
	fs::copy_file(source, destination, fs::copy_option::fail_if_exists);
#else
	struct stat st;
	if(::lstat(source.string().c_str(), &st) != 0) {
		throw SyscallException(errno, "lstat", source.string());
	}
	// Load source into memory
	MMap md(source.string());
	md.map();
	// Open temorary file near destination (we assume that
	// rename(tmpFile, destination) will succeed.
	const std::string tmpFile = destination.string() + ".tmp";
	FILE *f = fopen(tmpFile.c_str(), "w");
	if(f == NULL) {
		throw SyscallException(errno, "fopen", tmpFile);
	}
	try { // try to assure fclose is called!
		if(chmod(tmpFile.c_str(), st.st_mode & 0777) != 0) {
			std::cerr << SyscallException(errno, "chmod", tmpFile).what() << std::endl;
		}
		// copy the data
		if(fwrite(md.map(), md.len(), 1, f) != 1) {
			throw SyscallException(errno, "fwrite", source.string(), tmpFile);
		}
		fclose(f);
		f = NULL; // Make sure the file is not "closed" again in catch
		fs::rename(tmpFile, destination);
	} catch(...) {
		// Make sure f is closed and
		if(f != NULL) {
			fclose(f);
		}
		// remove the temporary file. Note: we do not check if the file
		// still exists, if it doesn't this will silently fail (no exception!).
		fs::remove(tmpFile);
		throw;
	}
#endif
	fs::remove(source);
}

void InstallItem::packReg(MXOSet &mxo) const {
	if(fs::is_regular_file(_absSourcePath) && areFilesEqual(_absSourcePath, _absInstalledPath)) {
		// Compare files, if equal thread as already copied
		mxo.addNote('E', "source exists and is equal", _absSourcePath, _absInstalledPath);
		if(!mxo.dryRun()) {
			// Here comes the same as in installLink
			fs::remove(_absInstalledPath);
			fs::create_symlink(_installContent, _absInstalledPath);
			mxo.setOwner(_absInstalledPath);
		}
	} else if(fs::is_symlink(_absSourcePath) || fs::exists(_absSourcePath)) {
		mxo.addError('J', "source exist, not a regular file or not equal", _absSourcePath, _absInstalledPath);
	} else {
		mxo.addNote('+', "source will be created", _absSourcePath, _absInstalledPath);
		// copy and link.
		if(!mxo.dryRun()) {
			fs::create_directory(_absSourcePath.parent_path());
			moveOrCopy(_absInstalledPath, _absSourcePath);
			// Here comes the same as in installLink
			fs::create_symlink(_installContent, _absInstalledPath);
			mxo.setOwner(_absInstalledPath);
		}
	}
}

void InstallItem::packLink(MXOSet &mxo) const {
	if(fs::is_symlink(_absSourcePath)) {
		const Path srcTarget = fs::read_symlink(_absSourcePath);
		if(srcTarget == _installContent) {
			mxo.addNote('-', "already exist with correct content", _absSourcePath, _absInstalledPath);
			mxo.noteStrangeOwner(_absSourcePath);
		} else if(!mxo.overwriteWrongLinks()) {
			mxo.addError('F', "exists with wrong content", _absSourcePath, _absInstalledPath, srcTarget);
		} else {
			mxo.addNote('R', "exists with wrong content, will be relinked", _absSourcePath, _absInstalledPath, srcTarget);
			if(!mxo.dryRun()) {
				fs::remove(_absSourcePath);
				fs::create_symlink(_installContent, _absSourcePath);
				mxo.setOwner(_absSourcePath);
			}
		}
	} else if(fs::is_regular_file(_absSourcePath) && _installContent == _absSourcePath)  {
		// Speciallity: if the file is already installed, that _absInstalledPath is a link into the
		// package version folders and the actuall file there is a regular file.
		mxo.addNote('n', "files seems to be installed already", _absSourcePath, _absInstalledPath, _installContent);
	} else if(fs::exists(_absSourcePath)) {
		mxo.addError('L', "destination exists, is no link", _absSourcePath, _absInstalledPath);
	} else {
		mxo.addNote('+', "source will be created", _absSourcePath, _absInstalledPath);
		if(!mxo.dryRun()) {
			fs::create_directory(_absSourcePath.parent_path());
			fs::create_symlink(_installContent, _absSourcePath);
			mxo.setOwner(_absSourcePath);
		}
	}
}

void InstallItem::uninstallLink(MXOSet &mxo) const {
	if(fs::is_symlink(_absInstalledPath)) {
		if(!mxo.isOwned(_absInstalledPath) && !mxo.overwriteForeignLinks()) {
			// Other owner, not instructed to _ignore this
			mxo.addError('P', "not owned by package user", _absSourcePath, _absInstalledPath);
		} else { // mxo.isOwned(_absInstalledPath) || mxo.overwriteForeignLinks()
			const Path dstLink = fs::read_symlink(_absInstalledPath);
			if(dstLink != _installContent && !mxo.overwriteWrongLinks()) {
				// _absInstalledPath's _installContent is not as expected
				mxo.addError('F', "exists with wrong content", _absSourcePath, _absInstalledPath, dstLink);
			} else {
				// _absInstalledPath's _installContent is as expected, or we are instructed to _ignore it
				mxo.addNote('+', "destination will be removed", _absSourcePath, _absInstalledPath, dstLink);
				if(!mxo.dryRun()) {
					fs::remove(_absInstalledPath);
				}
			}
		}
	} else if(fs::exists(_absInstalledPath)) {
		mxo.addError('L', "destination exists, is no link", _absSourcePath, _absInstalledPath);
	} else {
		mxo.addNote('-', "destination already uninstalled", _absSourcePath, _absInstalledPath);
	}
}

void InstallItem::installDir(MXOSet &mxo) const {
	if(fs::is_directory(_absInstalledPath)) {
		mxo.addNote('-', "already exist with correct content", _absSourcePath, _absInstalledPath);
	} else if(fs::exists(_absInstalledPath)) {
		mxo.addError('b', "destination exists, but isn't a folder", _absSourcePath, _absInstalledPath);
	} else {
		mxo.addNote('+', "destination will be created", _absSourcePath, _absInstalledPath);
		if(!mxo.dryRun()) {
			fs::create_directory(_absInstalledPath);
			mxo.setOwner(_absInstalledPath);
		}
	}
}

void InstallItem::packDir(MXOSet &mxo) const {
	if(fs::is_directory(_absSourcePath)) {
		mxo.addNote('-', "already exist with correct content", _absSourcePath);
	} else if(fs::exists(_absSourcePath)) {
		mxo.addError('e', "already exists but is not a dir", _absSourcePath);
	} else {
		mxo.addNote('+', "source will be created", _absSourcePath, _absInstalledPath);
		if(!mxo.dryRun()) {
			fs::create_directory(_absSourcePath);
			mxo.setOwner(_absSourcePath);
		}
	}
}

void InstallItem::uninstallDir(MXOSet &mxo) const {
	if(fs::is_directory(_absInstalledPath)) {
		if(!mxo.isOwned(_absInstalledPath)) {
			mxo.addNote('h', "destination not owned by package user, ignored", _absSourcePath, _absInstalledPath);
			return;
		}
		mxo.addNote('+', "destination will be removed", _absSourcePath, _absInstalledPath);
		if(!mxo.dryRun()) {
			try {
				fs::remove(_absInstalledPath);
			} catch(std::exception &err) {
				mxo.addNote('!', "removing failed, ignored", _absInstalledPath.string() + ": " + err.what());
			}
		}
	} else if(fs::exists(_absInstalledPath) || fs::is_symlink(_absInstalledPath)) {
		mxo.addError('b', "destination exists, but isn't a folder", _absSourcePath, _absInstalledPath);
	} else {
		mxo.addNote('-', "destination already uninstalled", _absSourcePath, _absInstalledPath);
	}
}

bool InstallItem::ignore(MXOSet &mxo) const {
	if(!_mapped) {
		mxo.addNote('M', "not mapped (see config)", _absSourcePath, _absInstalledPath);
		return true;
	} else if(_ignore) { // Must check this as last one, as !_mapped implies _ignore
		mxo.addNote('I', "ignored (see config)", _absSourcePath, _absInstalledPath);
		return true;
	}
	return false;
}

/**
 * General install/uninstall rules:
 * Directories are installed as directories (through mkdir). Existing directories or links to
 * existing directories are not changed and OK.
 * Directories are uninstalled with rmdir, but failures are reported as notices only.
 * That is because other packages may have installed files into a folder from this package.
 * If the installed object is not a directory or a link, it raises an error.
 *
 * Links are installed as copy of the link (so that source file and install file report the
 * smae string from readlink()).
 *
 * Files are installed as link to the source file.
 *
 * Ownership: is not changed on install for any existing object, is set to the package owner on
 * newly created objects. Existing objects other than directories with wrong ownership are reported.
 * If _ignore is set they are not reported.
 * On uninstall, objects with other owner (other than package user), are handle like this:
 * For directories the owner ship is ignored.
 * For any other files: if the owner is not the package user, an error is reported and no action
 * is done. If _ignore is set, the ownership is ignored (but reported). If force is set, the
 * action is taken regardless of the actual ownership.
 */
void InstallItem::install(MXOSet &mxo) const {
	if(ignore(mxo)) {
		return;
	}
	switch(_type) {
	case T_LINK:
		installLink(mxo);
		break;
	case T_DIR:
		installDir(mxo);
		break;
	case T_REG:
		installLink(mxo);
		break;
	default:
		mxo.addError('?', "unknown file type of source", _absSourcePath);
	}
}


void InstallItem::pack(MXOSet &mxo) const {
	if(ignore(mxo)) {
		return;
	}
	switch(_type) {
	case T_LINK:
		packLink(mxo);
		break;
	case T_DIR:
		packDir(mxo);
		break;
	case T_REG:
		packReg(mxo);
		break;
	default:
		mxo.addError('?', "unknown file type of source", _absInstalledPath);
	}
}

void InstallItem::uninstall(MXOSet &mxo) const {
	if(_keep) {
		mxo.addNote('K', "keeped (see config)", _absSourcePath, _absInstalledPath);
		return;
	} else if(ignore(mxo)) {
		return;
	}
	switch(_type) {
	case T_LINK:
		uninstallLink(mxo);
		break;
	case T_DIR:
		uninstallDir(mxo);
		break;
	case T_REG:
		uninstallLink(mxo);
		break;
	default:
		mxo.addError('?', "unknown file type of source", _absInstalledPath);
	}
}

void InstallItem::dumpInfo(std::ostream &stream) {
	stream << "^       file type ([l]ink, [d]ir, regular [f]ile, [o]ther)\n";
	stream << "  ^     installed (+) or not (-)\n";
	stream << "    ^   keep?\n";
	stream << "      ^ ignore?\n";
}

void InstallItem::dump(std::ostream &stream) const {
	stream << _type;
	if(_ignore) {
		stream << "  ";
	} else {
		switch(_type) {
			case T_LINK:
				if(fs::is_symlink(_absInstalledPath) && fs::read_symlink(_absInstalledPath) == _installContent) {
					stream << " +";
				} else {
					stream << " -";
				}
				break;
			case T_DIR:
				if(fs::is_directory(_absInstalledPath)) {
					stream << " +";
				} else {
					stream << " -";
				}
				break;
			case T_REG:
				if(fs::is_symlink(_absInstalledPath) && fs::read_symlink(_absInstalledPath) == _installContent) {
					stream << " +";
				} else {
					stream << " -";
				}
				break;
			default:
				stream << " ?";
				break;
		}
	}
	stream << " " << (_keep ? "K" : " ");
	stream << " " << (_ignore ? "I" : " ");
	stream << " " << _relSourcePath;
	stream << " -> " << _absInstalledPath;
	if(_mapped) {
		if(_type == T_LINK) {
			stream << " -> " << _installContent;
		}
	}
	stream << "\n";
}

InstallItem InstallItem::TSS::fromInstallPath(const Path &path) const {
	assert(path.is_absolute());
	InstallItem ii;
	ii._absInstalledPath = path;
	ii._relSourcePath = t.getSourcePath(path);
	assert(!ii._relSourcePath.is_absolute());
	ii._mapped = !ii._relSourcePath.empty();
	if(ii._mapped) {
		ii._absSourcePath = p.getVersionPath(v) / ii._relSourcePath;
	}
	assert(!ii._mapped || ii._absSourcePath.is_absolute());
	ii._type = InstallItem::getType(ii._absInstalledPath);
	if(ii._type == T_LINK) {
		ii._installContent = fs::read_symlink(ii._absInstalledPath);
	} else if(ii._type == T_REG) {
		ii._installContent = ii._absSourcePath;
	}
//	// if not _mapped, _relSourcePath is empty -> keepDir would be somehow useless
//	ii._keep = !ii._mapped || (ii._type == T_DIR && t.keepDir(ii._relSourcePath));
	// If not _mapped -> automaticly _ignore it
	ii._ignore = !ii._mapped || t.ignoreInSystem(ii._absInstalledPath);
	return ii;
}

InstallItem InstallItem::TSS::fromSourcePath(const Path &path) const {
	// path is a path relativ to vp (the package version folder: /packages/<pkg>/<ver>)
	const Path &vp = p.getVersionPath(v);
	assert(vp.is_absolute());
	assert(path.is_absolute());
	boost::filesystem::path::const_iterator itrVp = vp.begin();
	boost::filesystem::path::const_iterator itrPath = path.begin();
	// Find common base
	for( ; itrVp != vp.end() && itrPath != path.end() && *itrVp == *itrPath; ++itrVp, ++itrPath) {
	}
	if(itrVp != vp.end() || itrPath == path.end()) {
		throw std::runtime_error(path.string() + ": invalid source path (not part of root: " + vp.string() + ")");
	}
	// Now navigate down the directory branch
	Path rel_path;
	for( ; itrPath != path.end() ; ++itrPath) {
		rel_path /= *itrPath;
	}

	InstallItem ii;
	ii._relSourcePath = rel_path;
	ii._absSourcePath = path;
	ii._absInstalledPath = t.getInstallPath(ii._relSourcePath);
	ii._mapped = !ii._absInstalledPath.empty();
	assert(!ii._mapped || ii._absInstalledPath.is_absolute());
	ii._type = InstallItem::getType(ii._absSourcePath);
	if(ii._type == T_LINK) {
		ii._installContent = fs::read_symlink(ii._absSourcePath);
	} else if(ii._type == T_REG) {
		ii._installContent = ii._absSourcePath;
	}
	if(ii._type == T_DIR && ii._mapped) {
		ii._keep = t.keepDir(ii._relSourcePath);
	}
	ii._ignore = !ii._mapped || t.ignoreInSource(ii._relSourcePath);
	return ii;
}
