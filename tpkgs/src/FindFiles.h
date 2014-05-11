#pragma once

#include "File.h"
#include <string>
#include <set>
#include <sys/types.h>
#include <list>

#include "InstallItem.h"

class Package;
class Version;
class TPKGS;

class FindFiles {
public:
	typedef std::string Path;
	typedef std::set<Path> PSet;
	typedef dev_t DID;
	typedef std::list<InstallItem> IIList;

	typedef enum {
		PSA_INSTALL_DEVICE,
		PSA_MAPPED_DIRS,
		PSA_INSTALL_DIRS
	} PackSourceAlgorithm;

	typedef PackSourceAlgorithm PSA;

protected:
	InstallItem::TSS tss;

	IIList result;

	PSet toIgnore;

	bool ignore(const Path &path) const;

	static DID getDeviceID(const Path &path);

	void clear();

public:
	FindFiles(const Package &package, const Version &version, const TPKGS &tp);
	~FindFiles();

	IIList &getResult();

	void addToIgnore(const Path &path);

	static void getNewFiles(PSA psAlgo, const TPKGS &tp, const Package &p, const Version &v, IIList &iilist);
};

