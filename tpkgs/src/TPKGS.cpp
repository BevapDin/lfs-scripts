#include "TPKGS.h"
#include "File.h"
#include <iostream>
#include <cassert>
#include "Config.h"
#include <boost/filesystem/operations.hpp>
namespace fs = boost::filesystem;

TPKGS::TPKGS()
: _existingPackages()
, _arePackagesLoaded(false)
, _mainPackageDir("/packages")
, _installPathMap()
, _keepDirs()
, _ignoreInSource()
, _ignoreInSystem()
{
	_installPathMap.insert(PathPair("/", ""));
}

TPKGS::~TPKGS() {
}

bool TPKGS::ignoreInSource(const Path &path) const {
	return _ignoreInSource.count(path) > 0;
}

bool TPKGS::ignoreInSystem(const Path &path) const {
	return _ignoreInSystem.count(path) > 0;
}

bool TPKGS::keepDir(const Path &path) const {
	return _keepDirs.count(path) > 0;
}

void TPKGS::dump(std::ostream &stream) const {
	stream << "mainPackageDir: " << _mainPackageDir << "\n";
	stream << "installPathMap:\n";
	for(auto a = _installPathMap.begin(); a != _installPathMap.end(); a++) {
		stream << a->first << " -> " << a->second << "\n";
		if(!a->first.is_absolute()) {
			stream << "^^ source should be absolute!\n";
		}
		if(a->second.is_absolute()) {
			stream << "^^ destination should be relative!\n";
		}
	}
	stream << "keepDirs:\n";
	for(auto a = _keepDirs.begin(); a != _keepDirs.end(); a++) {
		stream << *a << "\n";
	}
	stream << "ignoreInSource:\n";
	for(auto a = _ignoreInSource.begin(); a != _ignoreInSource.end(); a++) {
		stream << *a << "\n";
		if(a->is_absolute()) {
			stream << "^^ should be relative!\n";
		}
	}
	stream << "ignoreInSystem:\n";
	for(auto a = _ignoreInSystem.begin(); a != _ignoreInSystem.end(); a++) {
		stream << *a << "\n";
		if(!a->is_absolute()) {
			stream << "^^ should be absolute!\n";
		}
	}
}

class TPKGS::TC : public Config {
public:
	TPKGS &tp;
	TC(TPKGS &tp) : tp(tp) { }
	virtual bool loadConfigEntry(const std::string &line);
};

bool TPKGS::TC::loadConfigEntry(const std::string &line) {
	if(loadSetting(line, "MAIN_PACKAGE_DIR", tp._mainPackageDir)) { return true; }
	if(loadSetting(line, "INSTALL_PATH_MAP", tp._installPathMap)) { return true; }
	if(loadSetting(line, "KEEP_DIRS", tp._keepDirs)) { return true; }
	if(loadSetting(line, "IGNORE_IN_SOURCE", tp._ignoreInSource)) { return true; }
	if(loadSetting(line, "IGNORE_IN_SYSTEM", tp._ignoreInSystem)) { return true; }
	return false;
}

void TPKGS::loadConfig(const Path &path) {
	TC tc(*this);
	tc.loadConfigOptional(path);
}

bool TPKGS::hasPackage(const PName &name) const {
	File proot(_mainPackageDir / name);
	return proot.isDir();
}

void TPKGS::loadConfig(const Package &package) {
	loadConfig(package.getConfigFile());
}

void TPKGS::loadConfig(const Package &package, const Version &version) {
	assert(!version.isEmptyVersion());
	loadConfig(package.getConfigFile(version));
}

void TPKGS::loadPackages() {
	File f(_mainPackageDir);
	for(File::dir_iterator a = f.begin(); a != f.end(); ++a) {
		if(a.isHidden() || a.isLink() || !a.isDir()) {
			continue;
		}
		const PName name = a.getFile();
		try {
			getPackage(name);
		} catch(std::exception &err) {
			std::cerr << "Package " << name << ": Error: " << err.what() << std::endl;
		}
	}
	_arePackagesLoaded = true;
}

Package &TPKGS::getPackage(const PName &name) {
	for(auto a = _existingPackages.begin(); a != _existingPackages.end(); a++) {
		Package &p = *a;
		if(p.getName() == name) {
			return p;
		}
	}
	Package p(name);
	p.init(*this);
	_existingPackages.push_back(p);
	return _existingPackages.back();
}

const TPKGS::PSet &TPKGS::getExistingPackages() const {
	if(!_arePackagesLoaded) {
		const_cast<TPKGS*>(this)->loadPackages();
	}
	return _existingPackages;
}

bool TPKGS::makeMapping(const Path &xSrc, const Path &xDst, const Path &path, size_t &lengthOfBestMatch, Path &result) const {
	fs::path::const_iterator itrVp = xSrc.begin();
	fs::path::const_iterator itrPath = path.begin();
	size_t nr = 0;
	for( ; itrVp != xSrc.end() && itrPath != path.end() && *itrVp == *itrPath; ++itrVp, ++itrPath) {
		nr += itrPath->string().length();
	}
	if(itrVp != xSrc.end()) { // mapping failed: xSrc is not a prefix of path
		return false;
	} else if(nr < lengthOfBestMatch && !result.empty()) { // already have a better result
		return false;
	}
	lengthOfBestMatch = nr;
	result = xDst;
	if(itrPath == path.end()) {
		return true;
	}
	for( ; itrPath != path.end() ; ++itrPath) {
		result /= *itrPath;
	}
	return false;
}

TPKGS::Path TPKGS::getSourcePath(const Path &installPath) const {
	assert(installPath.is_absolute());
	size_t lengthOfBestMatch = 0;
	Path result;
	for(auto a = _installPathMap.begin(); a != _installPathMap.end(); a++) {
		if(makeMapping(a->first, a->second, installPath, lengthOfBestMatch, result)) {
			break;
		}
	}
	return result;
}

TPKGS::Path TPKGS::getInstallPath(const Path &relSourcePath) const {
	assert(!relSourcePath.is_absolute());
	size_t lengthOfBestMatch = 0;
	Path result;
	// revers iteration to get shortest source dirs first
	for(auto a = _installPathMap.rbegin(); a != _installPathMap.rend(); a++) {
		if(makeMapping(a->second, a->first, relSourcePath, lengthOfBestMatch, result)) {
			break;
		}
	}
	return result;
}
