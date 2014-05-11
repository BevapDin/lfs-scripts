#pragma once

#include "FindFiles.h"

class TPKGS;
class Version;
class InstallItem;

#include "Package.h"
#include <iostream>
#include <map>
#include <boost/filesystem/path.hpp>

class DiffVersions {
public:
	typedef Package::IIList IIList;
	typedef boost::filesystem::path Path;
	typedef std::map<Path, const InstallItem*> PIIMap;
	typedef FindFiles::PackSourceAlgorithm PackSourceAlgorithm;

	typedef enum {
		EQUAL,
		EOF_ON_1,
		EOF_ON_2,
		DIFFERENT
	} CmpResult;

protected:
	const TPKGS &tp;
	const Package &p;

	IIList oldFiles;
	IIList newFiles;

	PIIMap oldIM;
	PIIMap newIM;

	std::ostream &stream;

	static void toPIIMap(PIIMap &map, const IIList &iilist);

	static CmpResult compare(const Path &file1, const Path &file2);
	static CmpResult compare(const InstallItem &ii, const InstallItem &oo);

	void listUnique(const PIIMap &map, const PIIMap &other) const;

	void run();

	void showDiff(const InstallItem &ii, const InstallItem &oo);
	void showEqual(const InstallItem &ii, const InstallItem &oo);

public:
	DiffVersions(const TPKGS &tp, const Package &p, std::ostream &stream);

	/**
	 * Compare vNew to vOld.
	 * Assumes the vNew to be not-jet-packed.
	 * vOld must exist.
	 */
	void run(const Version &vOld, const Version &vNew, PackSourceAlgorithm psAlgo);
	/**
	 * Same as run(vOld, Version(....), psAlgo)
	 */
	void run(const Version &vOld, PackSourceAlgorithm psAlgo);
	/**
	 * Compare vOld and vNew, both must exist.
	 */
	void run(const Version &vOld, const Version &vNew);
};

