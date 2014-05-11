#pragma once

#include <string>
#include <iostream>
#include <cassert>
#include <boost/filesystem/path.hpp>

#include "File.h"

class MXOSet;
class Package;
class Version;
class TPKGS;

class InstallItem {
public:
	typedef boost::filesystem::path Path;
	typedef enum {
		T_DIR,
		T_LINK,
		T_REG,
		T_OTHER,
		T_NON_EXIST
	} Type;

	class TSS;

protected:
	/**
	 * Path of source file (file in package dir), relativ to the
	 * version dir (e.g. relativ to '/packages/<pkg>/<version>/')
	 */
	Path _relSourcePath;
	/**
	 * Absolute path in package version dir (absolute path of #_relSourcePath).
	 */
	Path _absSourcePath;
	/**
	 * Absolute path of the installed file.
	 */
	Path _absInstalledPath;

	Type _type;
	/**
	 * If type is either T_LINK or T_REG (installation requires
	 * a link) this is set to the target of the installed link.
	 * Otherwise this is empty.
	 */
	Path _installContent;

	bool _keep;
	bool _ignore;
	bool _mapped;

	static Type getType(const File &file);

	/**
	 * Make a link to target on dst.
	 * If dst already exists and is a link to target nothing is done.
	 * If dst already exists and is a link, it may be relinked (depends
	 * on flags in mxo).
	 * Otherwise an error is set in mxo.
	 */
	void installLink(MXOSet &mxo) const;
	/**
	 * Reverts #installLink.
	 */
	void uninstallLink(MXOSet &mxo) const;
	void packLink(MXOSet &mxo) const;

	void installDir(MXOSet &mxo) const;
	void uninstallDir(MXOSet &mxo) const;
	void packDir(MXOSet &mxo) const;

	void packReg(MXOSet &mxo) const;

	bool ignore(MXOSet &mxo) const;

	InstallItem();

	friend class TSS;

public:
	~InstallItem();

	class TSS {
	public:
		const TPKGS &t;
		const Package &p;
		const Version &v;
		TSS(const TPKGS &t, const Package &p, const Version &v) : t(t), p(p), v(v) {
		}
		InstallItem fromSourcePath(const Path &sourcePath) const;
		InstallItem fromInstallPath(const Path &installPath) const;
	};

	const Path &getSourcePath() const { return _relSourcePath; }
	const Path &getSourceFile() const { return _absSourcePath; }
	const Path &getInstallFile() const { return _absInstalledPath; }
	const Path &getLinkContent() const { return _installContent; }
	bool is_ignore() const { return _ignore; }
	bool is_keep() const { return _keep; }
	bool is_mapped() const { return _mapped; }
	Type getType() const { return _type; }

	// Print content as single line in nice format
	void dump(std::ostream &stream) const;
	// Print footer for dump function output
	static void dumpInfo(std::ostream &stream);
	// actions that can be perfomred for an install item:
	void install(MXOSet &mxo) const;
	void uninstall(MXOSet &mxo) const;
	void pack(MXOSet &mxo) const;
};

inline std::ostream&operator<<(std::ostream &stream, InstallItem::Type type) {
	switch(type) {
		case InstallItem::T_DIR: return (stream << "d");
		case InstallItem::T_LINK: return (stream << "l");
		case InstallItem::T_REG: return (stream << "f");
		case InstallItem::T_OTHER: return (stream << "o");
		case InstallItem::T_NON_EXIST: return (stream << "-");
		default: assert(false); return stream;
	}
}

