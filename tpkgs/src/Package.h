#pragma once

#include <sys/types.h>

#include <string>
#include <set>
#include <list>
#include <stdexcept>
#include <boost/filesystem/path.hpp>

#include "Version.h"
#include "File.h"

class TPKGS;
class InstallItem;

/**
 *
 */
class Package {
public:
	typedef std::set<Version> VersionSet;
	typedef VersionSet VSet;
	typedef std::string Name;
	typedef uid_t UID;
	typedef gid_t GID;
	typedef std::list<InstallItem> InstallItemList;
	typedef InstallItemList IIList;
	typedef boost::filesystem::path Path;

	/**
	 * Throw by various functions when a non-existing version of this package
	 * is requested or required.
	 */
	class NoSuchVersionException : public std::runtime_error {
	public:
		NoSuchVersionException(const Package &p, const Version &v);
		virtual ~NoSuchVersionException() noexcept { }
	};

protected:
	/**
	 * The name of the package.
	 */
	Name _name;
	/**
	 * The package dir (/packaged/<pkg-name>/).
	 */
	Path _packageDir;
	/**
	 * All versions that are present in the package folder.
	 */
	VSet _existingVersions;
	/**
	 * The version that is currently marked as installed.
	 * This may be the empty version if no version is currently installed.
	 */
	Version _installedVersion;
	/**
	 * User ID of the package user.
	 */
	UID _userID;
	/**
	 * Group ID of the package user.
	 */
	GID _groupID;

	void initVersions();
	void initIDs();

public:
	Package(const Name &name);
	~Package();
	Package(const Package &) = default;
	Package&operator=(const Package &) = default;

	/**
	 * Initialize the object, that means load user and group ID,
	 * determine the existing versions and the installed version.
	 */
	void init(const TPKGS &tpkgs);
	/**
	 * Returns the package name.
	 */
	const Name &getName() const { return _name; }
	/**
	 * Returns the package dir.
	 */
	const Path &getPackageDir() const { return _packageDir; }
	/**
	 * Returns the user ID of the package user.
	 */
	UID getUserID() const { return _userID; }
	/**
	 * Returns the group ID of the package user.
	 */
	GID getGroupID() const { return _groupID; }
	/**
	 * Is this package currently installed?
	 */
	bool isInstalled() const;
	/**
	 * Is the given version currently installed?
	 */
	bool isInstalled(const Version &version) const;
	/**
	 * Is there at least one existing version of this package?
	 */
	bool hasVersions() const;
	/**
	 * Does the given version exist?
	 */
	bool hasVersion(const Version &version) const;
	/**
	 * Get the set of existing version.
	 */
	const VSet &getExistingVersions() const { return _existingVersions; }
	const VSet &getVersions() const { return _existingVersions; }
	/**
	 * Get the currently installed version or an empty version if
	 * no version is installed.
	 */
	Version getInstalledVersion() const { return _installedVersion; }
	/**
	 * Get the most recent existing version, or an empty version if
	 * there are no existing versions.
	 */
	Version getRecentVersion() const;
	/**
	 * Get the second most recent existing version, or an empty version if
	 * there are not two existing versions.
	 */
	Version getSecondRecentVersion() const;

	Version getInstalledOrRecentVersion() const;

	Path getVersionPath(const Version &version) const;

	void gatherInstallItemList(const Version &version, const TPKGS &tpkgs, IIList &list) const;

	inline bool operator<(const Package &other) const {
		return _name < other._name;
	}

	bool isOwned(const File &file) const;
	void setOwner(const File &file) const;

	void setInstalledVersion(const Version &version);
	void setUninstalled();

	Path getConfigFile() const;
	Path getConfigFile(const Version &version) const;

	/**
	 * @throws NoSuchVersionException if the version does not exist.
	 */
	void checkVersionExists(const Version &version) const;
};

