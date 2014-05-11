#pragma once

#include "FindFiles.h"

class FindFilesInMappedDirs : public FindFiles {
public:

protected:

public:
	FindFilesInMappedDirs(const Package &package, const Version &version, const TPKGS &tp);
	~FindFilesInMappedDirs();

	IIList &loadItems();

	bool operator()(const File::dir_iterator &a, const Path &p);
};

