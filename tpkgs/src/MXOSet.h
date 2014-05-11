#pragma once

#include <map>
#include <string>
#include <iostream>

#include "File.h"
#include <boost/filesystem/path.hpp>

class Version;
class Package;

class MXO {
public:
	char symbol;
	const char *name;
	bool error;
	mutable size_t count;
	MXO() { };
	MXO(char symbol, const char *name, bool error)
	: symbol(symbol)
	, name(name)
	, error(error)
	, count(1)
	{
	}
};

/**
 * Symbols:
 * '?' ... unhandled situation (e.g. dest is a regular file, but would be installed
 * as link or dir). Always an error.
 * '+' ... action is taken as supossed, without any problems. Always a note.
 * '-' ... no action needed, already fine. Always a note.
 */
class MXOSet : public std::map<char, MXO> {
public:
	typedef boost::filesystem::path Path;

protected:
	std::ostream *const verboseStream;
	std::ostream *const errorStream;
	int flags;

public:
	const Package &package;
	const Version &version;

#define FLAG(name, value) \
	enum { F_##name = (1 << value) }; \
	bool name() const { return (flags & F_##name) != 0; }

	FLAG(overwriteForeignLinks, 0);
	FLAG(overwriteWrongLinks, 1);
	FLAG(ignoreErrors, 2);
	FLAG(dryRun, 3);

	MXOSet(std::ostream *verboseStream, std::ostream *errorStream, const Package &package, const Version &version, int flags);

	void add(const MXO &mxo);
	bool hasError() const;

	void setOwner(const File &file) const;
	void noteStrangeOwner(const File &file);
	bool isOwned(const File &file) const;

	void printNotes(std::ostream &stream) const;

	void addNote(char symbol, const char *name, const Path &path);
	void addNote(char symbol, const char *name, const std::string &path);
	void addNote(char symbol, const char *name, const Path &link, const Path &target);
	void addNote(char symbol, const char *name, const Path &link, const Path &target, const Path &l);

	void addError(char symbol, const char *name, const Path &path);
	void addError(char symbol, const char *name, const std::string &path);
	void addError(char symbol, const char *name, const Path &link, const Path &target);
	void addError(char symbol, const char *name, const Path &link, const Path &target, const Path &l);
};
