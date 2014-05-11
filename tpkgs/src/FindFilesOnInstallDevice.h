#pragma once

#include "FindFiles.h"

class FindFilesOnInstallDevice : public FindFiles {
public:

protected:
	DID mainInstallPathDeviceID;

public:
	FindFilesOnInstallDevice(const Package &package, const Version &version, const TPKGS &tp);
	~FindFilesOnInstallDevice();

	IIList &loadItems();

	bool operator()(const File::dir_iterator &a, const Path &p);
};

