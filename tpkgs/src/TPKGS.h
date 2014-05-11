#pragma once

#include "Package.h"

#include <map>
#include <string>
#include <set>
#include <list>
#include <iostream>
#include <boost/filesystem/path.hpp>

class TPKGS {
public:
	typedef boost::filesystem::path Path;
	typedef Package::Name PName;
	typedef std::list<Package> PackageSet;
	typedef PackageSet PSet;
	typedef std::pair<Path, Path> PathPair;
	typedef std::set<PathPair> PathPairSet;
	typedef std::set<Path> PathSet;

	class TC;

protected:
	friend class TC;

	PSet _existingPackages;
	bool _arePackagesLoaded;

	Path _mainPackageDir;

	PathPairSet _installPathMap;
	PathSet _keepDirs;
	PathSet _ignoreInSource;
	PathSet _ignoreInSystem;

	void loadPackages();
	bool makeMapping(const Path &xSrc, const Path &xDst, const Path &sourcePath, size_t &lengthOfBestMatch, Path &result) const;

public:
	TPKGS();
	TPKGS(const TPKGS&) = delete;
	TPKGS& operator=(const TPKGS&) = delete;
	~TPKGS();

	Package &getPackage(const PName &name);

	bool hasPackage(const PName &name) const;

	const PSet &getExistingPackages() const;

	const Path &getMainPackageDir() const { return _mainPackageDir; }

	void loadConfig(const Path &file);
	void loadConfig(const Package &package);
	void loadConfig(const Package &package, const Version &version);

	bool keepDir(const Path &path) const;
	bool ignoreInSource(const Path &path) const;
	bool ignoreInSystem(const Path &path) const;

	Path getInstallPath(const Path &relSourcePath) const;
	Path getSourcePath(const Path &installPath) const;

	const PathPairSet &getInstallPathMap() const { return _installPathMap; }

	void dump(std::ostream &stream) const;
};

