#include "Config.h"

#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <map>
#include <set>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <string.h>
#include <boost/filesystem/operations.hpp>

#define WHITESPACE_STRING " \n\t\r"

Config::Config()
{
}

Config::~Config() {
}

std::string trim(const std::string &text) {
	const size_t i = text.find_first_not_of(WHITESPACE_STRING);
	const size_t j = text.find_last_not_of(WHITESPACE_STRING);
	if(i == std::string::npos) {
		assert(j == std::string::npos);
		return "";
	}
	if((i != 0) || (j == text.length() - 1)) {
		return text.substr(i, j - i + 1);
	}
	return text;
}

size_t Config::isSetting(const std::string &line, const std::string &name, const std::string &op) const {
	if(line.compare(0, name.length(), name) != 0) {
		return std::string::npos;
	}
	size_t i = line.find_first_not_of(WHITESPACE_STRING, name.length());
	if(i == std::string::npos) {
		if(line.compare(name.length(), op.length(), op) == 0) {
			return name.length() + op.length();
		}
		return std::string::npos;
	}
	if(line.compare(i, op.length(), op) != 0) {
		return std::string::npos;
	}
	size_t j = line.find_first_not_of(WHITESPACE_STRING, i + op.length());
	if(j == std::string::npos) {
		return i + op.length();
	}
	return j;
}

bool Config::loadSetting(const std::string &line, const std::string &name, std::string &var) const {
	size_t i = isSetting(line, name, "=");
	if(i == std::string::npos) {
		return false;
	}
	var = trim(line.substr(i));
	return true;
}

bool Config::loadSetting(const std::string &line, const std::string &name, Path &var) const {
	size_t i = isSetting(line, name, "=");
	if(i == std::string::npos) {
		return false;
	}
	var = trim(line.substr(i));
	return true;
}

bool Config::loadSetting(const std::string &line, const std::string &name, bool &var) const {
	const size_t i = isSetting(line, name, "=");
	if(i == std::string::npos) {
		return false;
	}
	const std::string value = line.substr(i);
	if(value == "true" || value == "1" || value == "on") {
		var = true;
	} else if(value == "false" || value == "0" || value == "off") {
		var = false;
	} else {
		throw std::runtime_error(std::string("can not parse as boolean: ") + value);
	}
	return true;
}

bool Config::loadSetting(const std::string &line, const std::string &name, Set &var) const {
	size_t i = isSetting(line, name, "=");
	if(i != std::string::npos) {
		var.clear();
		var.insert(
			trim(line.substr(i))
		);
		return true;
	}
	i = isSetting(line, name, "+=");
	if(i != std::string::npos) {
		var.insert(
			trim(line.substr(i))
		);
		return true;
	}
	i = isSetting(line, name, "-=");
	if(i != std::string::npos) {
		Set::iterator a = var.find(
			trim(line.substr(i))
		);
		if(a != var.end()) {
			var.erase(a);
		}
		return true;
	}
	return false;
}

bool Config::loadSetting(const std::string &line, const std::string &name, BSet &var) const {
	size_t i = isSetting(line, name, "=");
	if(i != std::string::npos) {
		var.clear();
		var.insert(
			trim(line.substr(i))
		);
		return true;
	}
	i = isSetting(line, name, "+=");
	if(i != std::string::npos) {
		var.insert(
			trim(line.substr(i))
		);
		return true;
	}
	i = isSetting(line, name, "-=");
	if(i != std::string::npos) {
		BSet::iterator a = var.find(
			trim(line.substr(i))
		);
		if(a != var.end()) {
			var.erase(a);
		}
		return true;
	}
	return false;
}

bool Config::loadSetting(const std::string &line, const std::string &name, PMap &var) const {
	size_t i = isSetting(line, name, "=");
	if(i != std::string::npos) {
		size_t j = line.find(':', i);
		if(j == std::string::npos) { return false; }
		var.clear();
		var.insert(PMap::value_type(
			trim(line.substr(i, j - i)),
			trim(line.substr(j + 1))
		));
		return true;
	}
	i = isSetting(line, name, "+=");
	if(i != std::string::npos) {
		size_t j = line.find(':', i);
		if(j == std::string::npos) { return false; }
		var.insert(PMap::value_type(
			trim(line.substr(i, j - i)),
			trim(line.substr(j + 1))
		));
		return true;
	}
	i = isSetting(line, name, "-=");
	if(i != std::string::npos) {
		size_t j = line.find(':', i);
		if(j == std::string::npos) { return false; }
		PMap::iterator a = var.find(PMap::value_type(
			trim(line.substr(i, j - i)),
			trim(line.substr(j + 1))
		));
		if(a != var.end()) {
			var.erase(a);
		}
		return true;
	}
	return false;
}

bool Config::loadSetting(const std::string &line, const std::string &name, PPMap &var) const {
	size_t i = isSetting(line, name, "=");
	if(i != std::string::npos) {
		size_t j = line.find(':', i);
		if(j == std::string::npos) { return false; }
		var.clear();
		var.insert(PPMap::value_type(
			trim(line.substr(i, j - i)),
			trim(line.substr(j + 1))
		));
		return true;
	}
	i = isSetting(line, name, "+=");
	if(i != std::string::npos) {
		size_t j = line.find(':', i);
		if(j == std::string::npos) { return false; }
		var.insert(PPMap::value_type(
			trim(line.substr(i, j - i)),
			trim(line.substr(j + 1))
		));
		return true;
	}
	i = isSetting(line, name, "-=");
	if(i != std::string::npos) {
		size_t j = line.find(':', i);
		if(j == std::string::npos) { return false; }
		PPMap::iterator a = var.find(PPMap::value_type(
			trim(line.substr(i, j - i)),
			trim(line.substr(j + 1))
		));
		if(a != var.end()) {
			var.erase(a);
		}
		return true;
	}
	return false;
}

void Config::loadConfigOptional(const Path &configFile) {
	boost::system::error_code ec;
	if(!boost::filesystem::exists(configFile, ec)) {
		return;
	}
	if(ec) {
		return;
	}
	loadConfig(configFile);
}

void Config::loadConfig(const Path &configFile) {
	std::ifstream input;
	input.exceptions(std::ios::badbit | std::ios::failbit);
	input.open(configFile.c_str(), std::ios::in);
	std::string line;
	input.exceptions(std::ios::iostate(0));
	while(!!(std::getline(input, line))) {
		loadLine(line);
	}
}


void Config::loadLine(const std::string &line_) {
	std::string line = line_;
	// Remove comments
	const size_t i = line.find('#');
	if(i != std::string::npos) {
		line.erase(i, line.length() - i);
	}
	// Remove whitespaces
	line = trim(line);
	if(line.empty()) {
		return;
	}
	if(!loadConfigEntry(line)) {
		throw std::runtime_error(std::string("Invalid line in config file: ") + line_);
	}
}

bool Config::loadConfigEntry(const std::string &line) {
	return false;
}

void Config::set(Set &a, const  Set &b, bool merge) {
	if(!merge) {
		a.clear();
		a = b;
		return;
	}
	for(Set::const_iterator i = b.begin(); i != b.end(); ++i) {
		a.insert(*i);
	}
}

void Config::set(PMap &a, const PMap &b, bool merge) {
	if(!merge) {
		a.clear();
		a = b;
		return;
	}
	for(PMap::const_iterator i = b.begin(); i != b.end(); ++i) {
		a.insert(*i);
	}
}
