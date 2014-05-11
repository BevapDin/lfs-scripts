#include "MXOSet.h"

#include <stdexcept>
#include "Package.h"

inline const char *TOBOOL(const bool b) {
	return b ? "true" : "false";
}

MXOSet::MXOSet(std::ostream *verboseStream, std::ostream *errorStream, const Package &p, const Version &v, int f)
: verboseStream(verboseStream)
, errorStream(errorStream)
, flags(f)
, package(p)
, version(v)
{
}

void MXOSet::add(const MXO& mxo) {
	const_iterator a = find(mxo.symbol);
	if(a == end()) {
		std::map<char, MXO>::insert(std::pair<char, MXO>(mxo.symbol, mxo));
	} else {
		a->second.count++;
		if(a->second.name != mxo.name) {
			throw std::runtime_error(std::string("MXO: differs: ") + std::string(1, mxo.symbol) + ": " + mxo.name + " <=> " + a->second.name);
		}
		if(a->second.error != mxo.error) {
			throw std::runtime_error(std::string("MXO: differs: ") + std::string(1, mxo.symbol) + ": " + TOBOOL(mxo.error) + " <=> " + TOBOOL(a->second.error));
		}
	}
}

bool MXOSet::hasError() const {
	for(const_iterator a = begin(); a != end(); a++) {
		if(a->second.error) {
			return true;
		}
	}
	return false;
}

void MXOSet::printNotes(std::ostream &stream) const {
	for(const_iterator a = begin(); a != end(); a++) {
		stream << std::string(1, a->second.symbol) << " ... ";
		if(a->second.error) {
			stream << "Error: ";
		}
		stream << a->second.name << " (" << a->second.count << ")\n";
	}
}

void MXOSet::addNote(char symbol, const char *name, const std::string &path) {
	if(dryRun()) {
		add(MXO(symbol, name, false));
	}
	if(verboseStream != 0 && dryRun()) {
		(*verboseStream) << std::string(1, symbol) << " " << path << "\n";
	}
}

void MXOSet::addNote(char symbol, const char *name, const Path &path) {
	addNote(symbol, name, path.string());
}

void MXOSet::addNote(char symbol, const char *name, const Path &link, const Path &target) {
	addNote(symbol, name, link.string() + " -> " + target.string());
}

void MXOSet::addNote(char symbol, const char *name, const Path &link, const Path &target, const Path &l) {
	if(target == l || link == l) {
		addNote(symbol, name, link.string() + " -> " + target.string());
	} else {
		addNote(symbol, name, link.string() + " -> " + target.string() + " [" + l.string() + "]");
	}
}

void MXOSet::addError(char symbol, const char *name, const std::string &path) {
	if(dryRun()) {
		add(MXO(symbol, name, true));
	}
	if(errorStream != 0 && dryRun()) {
		(*errorStream) << std::string(1, symbol) << " " << path << "\n";
	}
	if(!dryRun() && !ignoreErrors()) {
		throw std::runtime_error(path + ": " + name);
	}
}

void MXOSet::addError(char symbol, const char *name, const Path &path) {
	addError(symbol, name, path.string());
}

void MXOSet::addError(char symbol, const char *name, const Path &link, const Path &target) {
	addError(symbol, name, link.string() + " -> " + target.string());
}

void MXOSet::addError(char symbol, const char *name, const Path &link, const Path &target, const Path &l) {
	if(target == l || link == l) {
		addError(symbol, name, link.string() + " -> " + target.string());
	} else {
		addError(symbol, name, link.string() + " -> " + target.string() + " [" + l.string() + "]");
	}
}

void MXOSet::noteStrangeOwner(const File &file) {
	if(!isOwned(file)) {
		addNote('O', "Not owned by package user", file);
	}
}

bool MXOSet::isOwned(const File &file) const {
	return package.isOwned(file);
}

void MXOSet::setOwner(const File &file) const {
	package.setOwner(file);
}

