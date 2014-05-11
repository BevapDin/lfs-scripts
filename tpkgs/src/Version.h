#pragma once

#include <string>

/**
 * Package version. Versions are simple strings, composed of (hexa-)numeric
 * components separated by some character like '.' or '-'.
 */
class Version {
public:
	typedef std::string Name;

	static const Version EMPTY_VERSION;

protected:
	/**
	 * Name of the version.
	 */
	Name _name;

public:
	/**
	 * Creates an empty version.
	 */
	Version();
	/**
	 * Creates a named version.
	 */
	Version(const Name &name);
	~Version();

	/**
	 * Get the version string (or name) of the version.
	 */
	const Name &getName() const { return _name; }

	/**
	 * Is this the empty version?
	 */
	bool isEmpty() const { return _name.empty(); }
	bool isEmptyVersion() const { return isEmpty(); }

	inline bool operator==(const Version &other) const {
		return _name == other._name;
	}
	bool operator<(const Version &other) const;
};

