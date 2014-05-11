#pragma once

#include <stdexcept>
#include <sys/stat.h>

#include "File.h"
#include <set>
#include <string>
#include <iostream>

class RecursiveFileIterator {
public:
	typedef std::string Path;
	typedef std::set<Path> PathSet;

protected:
	PathSet folders;
	std::ostream *errorStream;

public:
	RecursiveFileIterator(std::ostream *errorStream = NULL)
	: folders()
	, errorStream(errorStream)
	{
	}
	void addPathToVisit(const Path &path) {
		folders.insert(path);
	}

	template<typename T>
	void run(T &t) {
		while(!folders.empty()) {
			const File f(*(folders.begin()));
			folders.erase(folders.begin());
			if(!f.isDir()) {
				continue;
			}
			try {
				for(File::dir_iterator a = f.begin(); a != f.end(); ++a) {
					const std::string path = a.getPath() + a.getFile();
					try {
						const bool b = t(a, path);
						if(a.isDir() && !a.isLink() && b) {
							folders.insert(path);
						}
					} catch(std::exception &err) {
						if(errorStream != NULL) {
							(*errorStream) << "Error for " << path << ": " << err.what() << "\n";
						}
					}
				}
			} catch(std::exception &err) {
				if(errorStream != NULL) {
					(*errorStream) << "Error for " << f.asString() << ": " << err.what() << "\n";
				}
			}
		}
	}
};

