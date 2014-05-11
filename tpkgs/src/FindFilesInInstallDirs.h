#pragma once

#include "FindFiles.h"

class FindFilesInInstallDirs : public FindFiles {
public:

protected:

public:
	FindFilesInInstallDirs(const Package &package, const Version &version, const TPKGS &tp);
	~FindFilesInInstallDirs();

	IIList &loadItems(const Path &installDirsFile);

	bool operator()(const File::dir_iterator &a, const Path &p);
};

