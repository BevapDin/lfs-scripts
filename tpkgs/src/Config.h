#pragma once

#include <string>
#include <map>
#include <set>
#include <boost/filesystem/path.hpp>

class Package;

class Config {
public:
	typedef boost::filesystem::path Path;
	typedef std::map<std::string, std::string> Map;
	typedef std::set<std::string> Set;
	typedef std::set<Path> BSet;
	typedef std::set<std::pair<std::string, std::string> > PMap;
	typedef std::set<std::pair<Path, Path> > PPMap;

protected:

	size_t isSetting(const std::string &line, const std::string &name, const std::string &op) const;

	bool loadSetting(const std::string &line, const std::string &name, std::string &var) const;
	bool loadSetting(const std::string &line, const std::string &name, Path &var) const;
	bool loadSetting(const std::string &line, const std::string &name, Set &var) const;
	bool loadSetting(const std::string &line, const std::string &name, BSet &var) const;
	bool loadSetting(const std::string &line, const std::string &name, PMap &var) const;
	bool loadSetting(const std::string &line, const std::string &name, PPMap &var) const;
	bool loadSetting(const std::string &line, const std::string &name, bool &var) const;
	bool loadSetting(const std::string &line, const std::string &name, int &var) const;
	bool loadSetting(const std::string &line, const std::string &name, double &var) const;
	bool loadSetting(const std::string &line, const std::string &name, float &var) const;

	virtual void loadLine(const std::string &line);
	virtual bool loadConfigEntry(const std::string &line);

	static void set(Set &a, const  Set &b, bool merge);
	static void set(PMap &a, const PMap &b, bool merge);

public:
	Config();
	virtual ~Config();

	/**
	 * Load configuration data from given file. Fails if the file is
	 * not accesable, of if it contains data that can not be understood.
	 */
	virtual void loadConfig(const Path &configFile);
	/**
	 * If the configuration file exists, load it.
	 */
	void loadConfigOptional(const Path &configFile);
};
