#include "TPKGS.h"
#include "InstallItem.h"
#include "File.h"
#include "MXOSet.h"
#include <stdlib.h>

#include <iostream>
#include <string.h>
#include <stdexcept>
#include <vector>
#include <cassert>
#include <fstream>

#include "FindFiles.h"
#include "DiffVersions.h"

#include "Popt.h"

typedef enum {
	A_NONE,
	A_CURRENT,
	A_INSTALLED,
	A_INSTALL,
	A_UNINSTALL,
	A_PACK,
	A_LIST_FILES,
	A_LIST_CONFIG,
	A_LIST_VERSIONS,
	A_DIFF_VERSIONS,
} Action;

typedef FindFiles::PackSourceAlgorithm PackSourceAlgorithm;
typedef std::vector<std::string> ArgList;
typedef TPKGS::PSet PSet;
typedef Package::VSet VSet;
typedef Package::IIList IIList;

static int verbose = 0;
static int dryRun = 0;
static const char *cacheFile = 0;
static PackSourceAlgorithm psAlgo = FindFiles::PSA_INSTALL_DIRS;

void listVersions(const Package &p, std::ostream &stream) {
	const VSet &vers = p.getExistingVersions();
	stream << p.getName() << ": " << vers.size() << " versions:";
	for(VSet::const_iterator b = vers.begin(); b != vers.end(); b++) {
		const Version &v = *b;
		stream << " " << v.getName();
		if(p.isInstalled(v)) {
			stream << " !";
		}
	}
	stream << "\n";
}

void listFiles(const TPKGS &tp, const Package &p, const Version &v, std::ostream &stream) {
	IIList iilist;
	p.gatherInstallItemList(v, tp, iilist);
	stream << "Package " << p.getName() << " - " << v.getName() << ":\n";
	for(IIList::const_iterator a = iilist.begin(); a != iilist.end(); a++) {
		const InstallItem &ii = *a;
		ii.dump(stream);
	}
	InstallItem::dumpInfo(stream);
}

void run(void (InstallItem::*Ptr)(MXOSet&)const, MXOSet &mxo, const IIList &iilist) {
	try {
		for(IIList::const_iterator a = iilist.begin(); a != iilist.end(); a++) {
			const InstallItem &ii = *a;
			if(mxo.ignoreErrors()) {
				try {
					(ii.*Ptr)(mxo);
				} catch(std::exception &err) {
					mxo.addError('#', "exception (ignored)", std::string(err.what()));
				}
			} else {
				(ii.*Ptr)(mxo);
			}
		}
	} catch(...) {
		mxo.printNotes(std::cout);
		throw;
	}
	mxo.printNotes(std::cout);
}

void pexec(const TPKGS &tp, Package &p, void (InstallItem::*Ptr)(MXOSet&)const, const Version &v, int flags, const IIList &iilist) {
	// First run for testing:
	{
		MXOSet mxo(
			(verbose || dryRun) ? &std::cout : NULL, // verbose stream
			&std::cerr, p, v, flags | MXOSet::F_dryRun
		);
		run(Ptr, mxo, iilist);
		if(dryRun) {
			return;
		}
		if(!mxo.ignoreErrors() && mxo.hasError()) {
			throw std::runtime_error("Dry run failed!");
		}
	}
	// second run for real:
	{
		MXOSet mxo(
			verbose ? &std::cout : NULL, // verbose stream
			&std::cerr, p, v, flags
		);
		run(Ptr, mxo, iilist);
	}
}

void loadFromCache(InstallItem::TSS &tss, const std::string &cache, IIList &iilist) {
	std::ifstream stream(cache.c_str(), std::ios::in);
	if(stream.is_open()) {
		std::string line;
		while(std::getline(stream, line)) {
			const InstallItem ii = tss.fromInstallPath(line);
			iilist.push_back(ii);
		}
	}
}

void saveToCache(const std::string &cache, const IIList &iilist) {
	std::ofstream stream(cache.c_str(), std::ios::out);
	if(stream.is_open()) {
		for(IIList::const_iterator a = iilist.begin(); a != iilist.end(); a++) {
			const InstallItem &ii = *a;
			stream << ii.getInstallFile().string() << "\n";
		}
	}
}

void pexec(const TPKGS &tp, Package &p, void (InstallItem::*Ptr)(MXOSet&)const, const Version &v, int flags) {
	IIList iilist;
	p.gatherInstallItemList(v, tp, iilist);
	pexec(tp, p, Ptr, v, flags, iilist);
}

void pexecU(const TPKGS &tp, Package &p, void (InstallItem::*Ptr)(MXOSet&)const, const Version &v, int flags) {
	IIList iilist;
	p.gatherInstallItemList(v, tp, iilist);
	pexec(tp, p, Ptr, v, flags, iilist);
	if(dryRun) {
		return;
	}
	// third run to uninstall folders
	{
		MXOSet mxo(
			&std::cout,
			&std::cerr, p, v, flags | MXOSet::F_ignoreErrors
		);
		run(Ptr, mxo, iilist);
	}
}

int xmain(int argc, char *argv[]) {
	Action action = A_NONE;
	int bypass_install_check = 0;
	int flagsA[3] = { 0, 0, 0 };
	struct poptOption optionsTable[] = {
		// Batch mode actions:
		{ "current", 'c', POPT_ARG_VAL, &action, A_CURRENT, "print the most recent version and exit with 0. If no versions exist, exit with 1", NULL },
		{ "installed", 'i', POPT_ARG_VAL, &action, A_INSTALLED, "print the currently installed version and exit with 0. If no version is installed, exit with 1", NULL },

		{ "cache-file", '\0', POPT_ARG_STRING, &cacheFile, 0, "cache file to cache files lists", 0 },

		// Active actions
		{ "install", '\0', POPT_ARG_VAL, &action, A_INSTALL, "install a spcific version of a package", NULL },
		{ "uninstall", '\0', POPT_ARG_VAL, &action, A_UNINSTALL, "uninstall a package", NULL },
		{ "pack", '\0', POPT_ARG_VAL, &action, A_PACK, "create a version from installed files", NULL },

		// Informational actions
		{ "files", '\0', POPT_ARG_VAL, &action, A_LIST_FILES, "list files of a package version", NULL },
		{ "diff", '\0', POPT_ARG_VAL, &action, A_DIFF_VERSIONS, "compare two versions of a package", NULL },
		{ "info", '\0', POPT_ARG_VAL, &action, A_LIST_VERSIONS, "list versions of packages (if none given, list all)", NULL },
		{ "config", '\0', POPT_ARG_VAL, &action, A_LIST_CONFIG, "show the configuration", NULL },
		// Flags
		{ "verbose", 'v', POPT_ARG_NONE, &verbose, 0, NULL, NULL },
		{ "dry-run", 'd', POPT_ARG_NONE, &dryRun, 0, "do not do anything, only show what would have been done, implies -v", NULL },
		{ "mapped-dirs", '\0', POPT_ARG_VAL, &psAlgo, FindFiles::PSA_MAPPED_DIRS, "use mapped dirs to gather files of the package", NULL },
		{ "install-device", '\0', POPT_ARG_VAL, &psAlgo, FindFiles::PSA_INSTALL_DEVICE, "use main install dir to gather files of the package", NULL },
		{ "bypass", '\0', POPT_ARG_NONE, &bypass_install_check, 0, "bypass install-version check for install, uninstall and pack actions", NULL },
		// Install flags:
		// Uninstall flags:
		// Common flags:
		{ "over-foreign", 'o', POPT_ARG_VAL, &(flagsA[0]), MXOSet::F_overwriteForeignLinks, "overwrite links that are not owned ny the package user", NULL },
		{ "over-wrong-target", 'f', POPT_ARG_VAL, &(flagsA[1]), MXOSet::F_overwriteWrongLinks, "overwrite links to wrong target", NULL },
		{ "ignore-errors", 'k', POPT_ARG_VAL, &(flagsA[2]), MXOSet::F_ignoreErrors, "ignore any errors and just keep going", NULL },

		POPT_AUTOHELP
		POPT_TABLEEND
	};

	TPKGS tp;
	tp.loadConfig("/etc/tpkgs2.conf");
	tp.loadConfig("tp.conf");
	Popt popt(tp);
	static const char *otherHelp = "\
You must specify one these actions:\n\
--install, --uninstall, --pack\n\
--files, --diff, --config\n\
--current, --installed\n\
\
";
	popt.init(argc, argv, optionsTable, otherHelp);
	int flags = 0;
	for(size_t i = 0; i < sizeof(flagsA) / sizeof(*flagsA); i++) {
		flags |= flagsA[i];
	}

	switch(action) {
	case A_LIST_CONFIG: {
		if(popt.hasMoreArgs()) {
			Version v;
			Package &p = popt.getPackage(v, Popt::NONE_NEEDED, false);
			tp.loadConfig(p);
			if(!v.isEmptyVersion()) {
				tp.loadConfig(p, v);
			}
		}
		popt.checkNoMoreArgs();
		tp.dump(std::cout);
	} break;
	case A_DIFF_VERSIONS: {
		Version vOld;
		Version vNew;
		Package &p = popt.getPackage(vOld, vNew);
		tp.loadConfig(p);
		popt.checkNoMoreArgs();
		// Diff of 2 specified versions: prog --diff <pkg> <old> <new>
		// if new does not exists, assume not-jet-packed for it.
		// Diff of 0 sepcified version: diff of recent ond second recent.
		DiffVersions dv(tp, p, std::cout);
		if(!p.hasVersions()) {
			throw std::runtime_error("package has no versions, needs at least one to compare");
		}
		if(vOld.isEmptyVersion()) {
			assert(vNew.isEmptyVersion());
			// No version given at all.
			if(p.getVersions().size() < 2) {
				throw std::runtime_error("package has fewer than 2 versions, must explicitly define which to compare");
			}
			vOld = p.getSecondRecentVersion();
			vNew = p.getRecentVersion();
			dv.run(vOld, vNew);
		} else if(vNew.isEmptyVersion()) {
			// A vOld is give, but no vNew
			if(p.hasVersion(vOld)) {
				// Only one, existing version given, compare to not-jet-installed.
				dv.run(vOld, psAlgo);
			} else {
				// Only one, non-existing version given, assume this to be the not-jet-installed one
				// and compare it to the recent or installed one.
				vNew = vOld;
				vOld = p.getInstalledOrRecentVersion();
				assert(!vOld.isEmptyVersion()); // See above assert, that the package has any versions
				dv.run(vOld, vNew, psAlgo);
			}
		} else {
			// OK, two versions given, the first one must exist.
			assert(!vOld.isEmptyVersion());
			assert(!vNew.isEmptyVersion());
			p.checkVersionExists(vOld);
			if(p.hasVersion(vNew)) {
				dv.run(vOld, vNew);
			} else {
				dv.run(vOld, vNew, psAlgo);
			}
		}
	} break;
	case A_PACK: {
		Version v;
		Package &p = popt.getPackage2(v);
		tp.loadConfig(p);
		popt.checkNoMoreArgs();
		if(!bypass_install_check && p.isInstalled()) {
			throw std::runtime_error(p.getName() + ": there is already a version installed");
		}
		if(v.isEmptyVersion()) {
			v = Version(popt.popArg("missing version"));
		}
		if(v.isEmptyVersion()) {
			throw std::runtime_error("version string must not be empty");
		}
		tp.loadConfig(p, v);
		IIList iilist;
		if(cacheFile != 0) {
			InstallItem::TSS tss(tp, p, v);
			loadFromCache(tss, cacheFile, iilist);
		}
		if(iilist.empty()) {
			FindFiles::getNewFiles(psAlgo, tp, p, v, iilist);
			if(cacheFile != 0) {
				saveToCache(cacheFile, iilist);
			}
		}
		pexec(tp, p, &InstallItem::pack, v, flags, iilist);
		if(dryRun) {
			break;
		}
		p.setInstalledVersion(v);
	} break;
	case A_CURRENT: {
		int ret = 0;
		do {
			Package &p = popt.getPackage();
			if(p.hasVersions()) {
				const Version v = p.getRecentVersion();
				std::cout << v.getName() << std::endl;
			} else {
				ret |= 1;
			}
		} while(popt.hasMoreArgs());
		return ret;
	} break;
	case A_INSTALLED: {
		int ret = 0;
		do {
			Package &p = popt.getPackage();
			if(p.isInstalled()) {
				const Version v = p.getInstalledVersion();
				std::cout << v.getName() << std::endl;
			} else {
				ret |= 1;
			}
		} while(popt.hasMoreArgs());
		return ret;
	} break;
	case A_INSTALL: {
		Version v;
		Package &p = popt.getPackage(v, Popt::CURRENT, true);
		tp.loadConfig(p);
		popt.checkNoMoreArgs();
		if(!bypass_install_check && p.isInstalled(v)) {
			throw std::runtime_error(p.getName() + ": version " + v.getName() + " is already installed");
		} else if(!bypass_install_check && p.isInstalled()) {
			throw std::runtime_error(p.getName() + ": there is already another version installed");
		}
		tp.loadConfig(p, v);
		pexec(tp, p, &InstallItem::install, v, flags);
		if(dryRun) {
			break;
		}
		p.setInstalledVersion(v);
	} break;
	case A_UNINSTALL: {
		Version v;
		Package &p = popt.getPackage(v, Popt::INSTALLED, true);
		tp.loadConfig(p);
		popt.checkNoMoreArgs();
		if(!bypass_install_check && !p.isInstalled(v)) {
			throw std::runtime_error(p.getName() + ": version " + v.getName() + " is not installed");
		}
		tp.loadConfig(p, v);
		pexecU(tp, p, &InstallItem::uninstall, v, flags);
		if(dryRun) {
			break;
		}
		p.setUninstalled();
	} break;
	case A_LIST_VERSIONS: {
		if(popt.hasMoreArgs()) {
			while(popt.hasMoreArgs()) {
				const Package &p = popt.getPackage();
				listVersions(p, std::cout);
			}
		} else {
			const auto &pkgs = tp.getExistingPackages();
			for(auto a = pkgs.begin(); a != pkgs.end(); a++) {
				const Package &p = *a;
				listVersions(p, std::cout);
			}
		}
	} break;
	case A_LIST_FILES: {
		Version v;
		const Package &p = popt.getPackage(v, Popt::INSTALLED_OR_CURRENT, true);
		tp.loadConfig(p);
		tp.loadConfig(p, v);
		listFiles(tp, p, v, std::cout);
	} break;
	default:
		throw std::runtime_error("missing action on command line");
	}
	return 0;
}

int main(int argc, char *argv[]) {
	try {
		File::setLogDiskAccess(false);
		return xmain(argc, argv);
	} catch(std::exception &err) {
		std::cerr << "Error: " << err.what() << std::endl;
		return 1;
	}
}
