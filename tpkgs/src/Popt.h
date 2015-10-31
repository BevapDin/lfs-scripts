#pragma once

#include <popt.h>
#include <vector>
#include <string>
#include <stdexcept>

class TPKGS;
class Version;
class Package;

class Popt {
public:
	typedef std::vector<std::string> ArgList;

	typedef enum {
		// Apply no default, use the one given on the command line, if non is given, throw an exception
		NO_DEFAULT,
		// use the installed one as default, if no version is installed, like NO_DEFAULT
		INSTALLED,
		// use the most recent one as default, if no version exists at all, like NO_DEFAULT
		CURRENT,
		// use the installed one or (if none is installed) use the most recent one as default,
		// if no version exists at all, like NO_DEFAULT
		INSTALLED_OR_CURRENT,
		// Do not extract a version, getPackage may still extract a version if the package name
		// contains the version directly as in <pkg>:<version>
		// Also if this is supplied to #getVersion, the version is extracted if there is a
		// argument left, but it will not trigger an error if there are no more arguments to extract
		// the version from.
		NONE_NEEDED,
	} VersionFlags;
	typedef VersionFlags VersionFlag;

	class MissingArgumentException : public std::runtime_error {
	public:
		MissingArgumentException(const std::string &msg) : std::runtime_error(msg) {
		}
		virtual ~MissingArgumentException() throw() {
		}
	};

public:
	poptContext _optCon;
	ArgList _args;
	TPKGS &_tp;

	static void removePrefix(std::string &s, const std::string &prefix);

	void clear();

public:
	Popt(TPKGS &tp);
	~Popt();

	/**
	 * Get the next argument on the list of arguments (if any), remove it
	 * from the list of arguments and return it.
	 * If there is no argument at all, throw an exception.
	 */
	std::string popArg();
	/**
	 * Get the next argument on the list of arguments (if any), remove it
	 * from the list of arguments and return it.
	 * If there is no argument at all, throw an exception with the supplied
	 * error message.
	 */
	std::string popArg(const std::string &errorString);
	/**
	 * Number of unprocessed arguments.
	 */
	size_t getArgCount() const;

	void init(int argc, char *argv[], struct poptOption *optionsTable, const char *oh = "");
	void usage();
	void usage(const std::string &message);

	// Extract a package name, no version is need
	Package &getPackage();
	/**
	 * Get package and version, both are mandatory. This function interprets a single argument
	 * as the version, not as the package name.
	 */
	Package &getPackage2(Version &v);
	/**
	 * Extract a version from the package name, if it is in the form <pkg-name>:<version>.
	 * @returns true if the version has been extracted, false otherwise.
	 */
	bool extractPackageAndVersionFromString(std::string &pname, Version &v) const;
	static std::string getDefaultPackageName();
	void warnIfPackageExists(const std::string &pname, const std::string &oname) const;
	// Extract package and version, both may be in on argument as
	// <pkg>:<version> or they may be in two arguments.
	// @param versionMustExist If true, the function checks that the version supplied
	// actually exists, otherwise it throws an exception.
	Package &getPackage(Version &v, VersionFlags vf, bool versionMustExist);
	Package &getPackage(Version &v1, Version &v2);
	bool hasMoreArgs() const;
	/**
	 * Extract the version.
	 * The #VersionFlag for its meaning.
	 * The functions may throw, see popArg.
	 * The returned version is either empty if vf==NON_NEEDED and no more arguemnts are there.
	 * Otherwise the returned version is choosen from the package (according to vf)
	 * or taken from the command line. In the later case the version is check to exist, if request
	 * per versionMustExist parameter.
	 */
	Version getVersion(const Package &p, VersionFlag vf, bool versionMustExist);

	void checkNoMoreArgs() const;
};

