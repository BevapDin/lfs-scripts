#include "MMap.h"

// for errors
#include <errno.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <system_error>

// for mmap
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
// for max()
#include <limits>

#include <cassert>

MMap::MMap()
: _filename()
, _length(std::numeric_limits<decltype(_length)>::max())
, _addr(nullptr)
, _fd(-1)
{
}

MMap::MMap(const std::string &filename)
: _filename(filename)
, _length(std::numeric_limits<decltype(_length)>::max())
, _addr(nullptr)
, _fd(-1)
{
}

MMap::~MMap() throw() {
	try {
		close();
	} catch(std::exception &err) {
		try {
			std::cerr << "Error: " << err.what() << std::endl;
		} catch(...) {
			// Ignored - no way of doing anything here
		}
	}
}

void MMap::close() {
	if(_addr != nullptr) {
		const int r = ::munmap(_addr, _length);
		assert(r == 0); (void) r;
		_addr =  nullptr;
	}
	if(_fd != -1) {
		const int r = ::close(_fd) != 0;
		assert(r == 0); (void) r;
		_fd = -1;
	}
	_length = std::numeric_limits<decltype(_length)>::max();
}

const std::string &MMap::getFilename() const {
	return _filename;
}

void MMap::setFilename(const std::string &filename) {
	close();
	_filename = filename;
}

size_t MMap::len() {
	if(_length != std::numeric_limits<decltype(_length)>::max()) {
		return _length;
	}
	struct stat sb;
	if(::stat(_filename.c_str(), &sb) != 0) {
		throw std::system_error(std::error_code(errno, std::system_category()), "can not fstat");
	}
	if(!S_ISREG(sb.st_mode)) {
		throw std::runtime_error("invalid file type, not a regular file");
	}
	const auto l = static_cast<decltype(_length)>(sb.st_size);
	if(static_cast<decltype(sb.st_size)>(l) != sb.st_size) {
		// Overflow! Can't map anyway
		throw std::runtime_error(_filename + ": can not mmap - too big (does not fit into size_t)");
	}
	_length = l;
	return _length;
}

void MMap::open() {
	map();
}

const void *MMap::map() {
	if(_addr != nullptr) {
		return _addr;
	}
	if(len() == 0) {
		return nullptr;
	}
	if(_fd == -1) {
		_fd = ::open(_filename.c_str(), O_RDONLY);
		if(_fd == -1) {
			throw std::system_error(std::error_code(errno, std::system_category()), "fopen failed");
		}
	}
	assert(_addr == nullptr);
	auto addr = ::mmap(nullptr, _length, PROT_READ, MAP_PRIVATE, _fd, 0);
	if(addr == MAP_FAILED) {
		throw std::system_error(std::error_code(errno, std::system_category()), "mmap failed");
	}
	_addr = addr;
	assert(_addr != nullptr);
	return _addr;
}
