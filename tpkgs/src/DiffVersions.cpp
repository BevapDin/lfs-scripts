#include "DiffVersions.h"
#include "TPKGS.h"
#include "InstallItem.h"
#include "MMap.h"

#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <cassert>
#include <boost/filesystem/operations.hpp>
namespace fs = boost::filesystem;

DiffVersions::DiffVersions(const TPKGS &tp, const Package &p, std::ostream &stream)
: tp(tp)
, p(p)
, stream(stream)
{
}

void DiffVersions::toPIIMap(PIIMap &map, const IIList &iilist) {
	for(IIList::const_iterator a = iilist.begin(); a != iilist.end(); a++) {
		const InstallItem &ii = *a;
		const Path &path = ii.getSourcePath();
		assert(map.count(path) == 0);
		map.insert(PIIMap::value_type(path, &ii));
	}
}

void DiffVersions::listUnique(const PIIMap &map, const PIIMap &other) const {
	for(PIIMap::const_iterator a = map.begin(); a != map.end(); a++) {
		const InstallItem &ii = *(a->second);
		const PIIMap::const_iterator b = other.find(a->first);
		if(b != other.end()) {
			continue;
		}
		// Hack: the gather new files function (which searches the global system directories
		// for newly insatlled files) does not report directories, that are not owned by
		// the package user (like /usr/bin).
		// The gatherIIList function of Package lists those directories because they are in
		// the version dir (/packages/<pkg>/<version>) and must therefor be installed.
		// Therfor directories like /usr/lib would be listed as "not in new version".
		// If they exists, we simply assume that they would became part of the new
		// version.
		if(&map == &oldIM && ii.getType() == InstallItem::T_DIR && fs::exists(ii.getInstallFile())) {
			continue;
		}
		stream << " " << ii.getType() << "  " << ii.getSourcePath() << "\n";
	}
}

void DiffVersions::run(const Version &vOld, const Version &vNew, PackSourceAlgorithm psAlgo) {
	if(vNew == vOld) {
		throw std::runtime_error("can not compare the same version");
	}
	p.gatherInstallItemList(vOld, tp, oldFiles);
	FindFiles::getNewFiles(psAlgo, tp, p, vNew, newFiles);
	run();
}

void DiffVersions::run(const Version &vOld, PackSourceAlgorithm psAlgo) {
	run(vOld, Version("new"), psAlgo);
}

void DiffVersions::run(const Version &vOld, const Version &vNew) {
	if(vNew == vOld) {
		throw std::runtime_error("can not compare the same version");
	}
	p.gatherInstallItemList(vOld, tp, oldFiles);
	p.gatherInstallItemList(vNew, tp, newFiles);
	run();
}

DiffVersions::CmpResult DiffVersions::compare(const Path &file1, const Path &file2) {
	MMap m1(file1.string());
	MMap m2(file2.string());
	if(m1.len() < m2.len()) {
		if(memcmp(m1.map(), m2.map(), m1.len()) == 0) {
			return EOF_ON_1;
		} else {
			return DIFFERENT;
		}
	} else if(m1.len() > m2.len()) {
		if(memcmp(m1.map(), m2.map(), m2.len()) == 0) {
			return EOF_ON_2;
		} else {
			return DIFFERENT;
		}
	} else {
		assert(m1.len() == m2.len());
		if(memcmp(m1.map(), m2.map(), m1.len()) == 0) {
			return EQUAL;
		} else {
			return DIFFERENT;
		}
	}
}

DiffVersions::CmpResult DiffVersions::compare(const InstallItem &ii, const InstallItem &oo) {
	if(fs::exists(oo.getSourceFile())) {
		return compare(ii.getSourceFile(), oo.getSourceFile());
	} else {
		return compare(ii.getSourceFile(), oo.getInstallFile());
	}
}

void DiffVersions::showEqual(const InstallItem &ii, const InstallItem &oo) {
	stream << " " << ii.getType() << "  ";
	stream << " " << ii.getSourcePath() << "\n";
}

void DiffVersions::showDiff(const InstallItem &ii, const InstallItem &oo) {
	if(ii.getType() != oo.getType()) {
		stream << "!" << ii.getType() << " " << oo.getType();
		stream << " " << ii.getSourcePath() << "\n";
		return;
	}
	switch(ii.getType()) {
		case InstallItem::T_DIR:
		case InstallItem::T_OTHER:
		case InstallItem::T_NON_EXIST:
			// we can not compare those types:
			// Dirs can bot compared, they exist, or not.
			// Other types are unknown at all.
			// Non-existing things can not be compared per definition.
			showEqual(ii, oo);
			break;
		case InstallItem::T_LINK:
			if(ii.getLinkContent() == oo.getLinkContent()) {
				showEqual(ii, oo);
			} else {
				stream << "~" << ii.getType() << "  ";
				stream << " " << ii.getSourcePath() << " ";
				stream << " " << ii.getLinkContent() << " <=> " << oo.getLinkContent() << "\n";
			}
			break;
		case InstallItem::T_REG:
			switch(compare(ii, oo)) {
				case EQUAL:
					showEqual(ii, oo);
					break;
				case DIFFERENT:
					stream << "~" << ii.getType() << "  ";
					stream << " " << ii.getSourcePath() << "\n";
					break;
				case EOF_ON_1:
					stream << "+" << ii.getType() << "  ";
					stream << " " << ii.getSourcePath() << "\n";
					break;
				case EOF_ON_2:
					stream << "-" << ii.getType() << "  ";
					stream << " " << ii.getSourcePath() << "\n";
					break;
				default:
					assert(false);
			}
			break;
		default:
			assert(false);
			return;
	}
}

void DiffVersions::run() {
	toPIIMap(newIM, newFiles);
	toPIIMap(oldIM, oldFiles);
	stream << oldFiles.size() << " old, " << newFiles.size() << " new, those are in both:\n";
	for(PIIMap::const_iterator a = oldIM.begin(); a != oldIM.end(); a++) {
		const InstallItem &ii = *(a->second);
		const PIIMap::const_iterator b = newIM.find(a->first);
		if(b == newIM.end()) {
			continue;
		}
		const InstallItem &nn = *(b->second);
		showDiff(ii, nn);
	}
	stream << "^ different type (!), different content (~), newer is smaller (-), newer is greater (+)\n";
	stream << "Those are only in the old version:\n";
	listUnique(oldIM, newIM);
	stream << "Those are only in the new version:\n";
	listUnique(newIM, oldIM);
}

