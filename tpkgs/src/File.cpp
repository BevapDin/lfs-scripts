#include "File.h"

#include <string>
#include <string.h>
#include <stdexcept>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <memory.h>

SyscallException::SyscallException(int errNumber, const char *syscall, const std::string &path1, const std::string &path2)
: std::exception()
, errNumber(errNumber)
, syscall(syscall)
, path1(path1)
, path2(path2)
, message()
{
}

SyscallException::SyscallException(int errNumber, const char *syscall, const std::string &path1)
: std::exception()
, errNumber(errNumber)
, syscall(syscall)
, path1(path1)
, path2()
, message()
{
}

SyscallException::~SyscallException() throw() {
}

const char* SyscallException::what() const throw() {
	if(message.empty()) {
		formatMessage();
	}
	return message.c_str();
}

void SyscallException::formatMessage() const {
	std::ostringstream buffer;
	buffer << "Error " << errNumber << " on " << syscall << "(";
	if(!path1.empty()) {
		buffer << "\"" << path1 << "\"";
		if(!path2.empty()) {
			buffer << ", \"" << path2 << "\"";
		}
	}
	buffer << "): " << strerror(errNumber);
	message = buffer.str();
}

using namespace std;

bool File::logDiskAccess(true);

File::File()
: fileAsString(".", 1)
{
}

File::File(const File &other)
: fileAsString(other.fileAsString)
{
	if(fileAsString.empty()) {
		fileAsString.assign(".", 1);
	}
}

File::File(const string &a)
: fileAsString(a)
{
	cheapNormalize();
}

File::File(bool, const string &a)
: fileAsString(a)
{
}

File::File(const char *a)
: fileAsString(a)
{
	cheapNormalize();
}

File::File(const File &absolutePath, const string &relativeFile)
: fileAsString(absolutePath)
{
	if(relativeFile.empty()) {
		return;
	}
	if(relativeFile.at(0) == '/') {
		fileAsString = relativeFile;
		cheapNormalize();
		return;
	}
	if(!absolutePath.isAbsolutePath()) {
		throw runtime_error(string("Absolute path is not absolute") + absolutePath.fileAsString);
	}
	if(fileAsString.at(fileAsString.length() - 1) != '/') {
		fileAsString.append("/", 1);
	}
	File tmp(relativeFile);
	string &file = tmp.fileAsString;

	size_t i = 0;
	while(i < file.length()) {
		if(file.at(i) == '/') {
			i++;
			continue;
		}
		if(i + 1 == file.length() && file.compare(i, 1, ".") == 0) {
			break;
		}
		if(i + 2 == file.length() && file.compare(i, 2, "..") == 0) {
			break;
		}
		if(file.compare(i, 2, "./") == 0) {
			i += 2;
			continue;
		}
		if(file.compare(i, 3, "../") == 0) {
			i += 3;
			if(fileAsString.length() == 1) {
				assert(fileAsString.at(0) == '/');
				continue;
			}
			size_t a = fileAsString.find_last_of('/', fileAsString.length() - 2);
			assert(a != string::npos);
			fileAsString.erase(a + 1, fileAsString.length() - a - 1);
			continue;
		}
		size_t a = file.find('/', i);
		if(a == string::npos) {
			fileAsString.append(file, i, file.length() - i);
			break;
		}
		a++;
		fileAsString.append(file, i, a - i);
		i = a;
	}
	assert(!fileAsString.empty());
}

File::File(const dir_iterator &it)
: fileAsString(it.getPath() + "/" + it.getFile())
{
	cheapNormalize();
}

File::~File() {
}

File &File::operator=(const File &other) {
	if(this == &other) {
		return *this;
	}
	fileAsString = other.fileAsString;
	return *this;
}

File &File::operator=(const string &other) {
	if(fileAsString == other) {
		return *this;
	}
	const File tmp(other);
	return ((*this) = tmp);
}

bool File::operator==(const File &other) const {
	if(this == &other) {
		return true;
	}
	return fileAsString.compare(other.fileAsString) == 0;
}

bool File::operator==(const string &other) const {
	return fileAsString.compare(other) == 0;
}

bool File::operator<(const File &other) const {
	return fileAsString.compare(other.fileAsString) < 0;
}

bool File::operator<(const string &other) const {
	return fileAsString.compare(other) < 0;
}

#if defined (DEBUG) || defined(_DEBUG)
#define xerase(a, b) cout << "Erase '" << fileAsString.substr(0, a) << "<[" << fileAsString.substr(a, b) << "]>" << fileAsString.substr((a) + (b)) << "'\n"; fileAsString.erase(a, b)
#else
#define xerase(a, b) fileAsString.erase(a, b)
#endif

void File::cheapNormalize() {
	if(fileAsString.empty()) {
		fileAsString.assign(".", 1);
		return;
	}
	while(fileAsString.length() > 2 && fileAsString.compare(0, 2, "./") == 0) {
		xerase(0, 2);
	}
	size_t i = fileAsString.find('/');
	while(i != string::npos) {
		assert(fileAsString.at(i) == '/');
		if(i + 1 == fileAsString.length()) { // End of string
			break;
		}
		if(fileAsString.compare(i + 1, 2, "./") == 0) {
			// Sequence '/./' found -> replace with '/'
			xerase(i, 2);
			// No need to change i, still points to a '/'
			continue;
		}
		if(fileAsString.compare(i + 1, 3, "../") == 0) {
			// Sequence '/../' found
			if(i == 0) {
				// fileAsString _starts_ with '/../' --> is absolute path!
				xerase(0, 3);
				// No need to change i, still points to a '/'
				continue;
			}
			size_t k = fileAsString.find_last_of('/', i - 1);
			if(k == string::npos) {
				// somthing like 'abc/../dde'
				if(i == 2 && fileAsString.compare(0, 2, "..") == 0) {
					// Is '../../dde'
					i = fileAsString.find('/', i + 1);
				} else if(i == 1 && fileAsString.compare(0, 1, ".") == 0) {
					// Is './../dde'
					assert(false); // Should not happen, './' is removed at begin
					xerase(0, 2); // 2 <=> './'
					i = fileAsString.find('/');
				} else {
					xerase(0, i + 4); // 4 <=> '/../'
					i = fileAsString.find('/');
				}
				continue;
			}
			xerase(k + 1, i - k + 3); // (i-k) <=> 'abc/' + 3 <=> '../' --> replace 'abc/../'
			i = k;
			continue;
		}
		size_t j = fileAsString.find_first_not_of('/', i + 1);
		if(j == string::npos) {
			// No next non-'/', but i is not the last charachter, so more '/' follow
			xerase(i + 1, fileAsString.length() - i - 1);
			break;
		}
		if(j == i + 1) {
			i = fileAsString.find('/', j);
			continue;
		}
		assert(j > i + 1);
		xerase(i + 1, j - i - 1);
		assert(i + 1 < fileAsString.length());
		assert(fileAsString.at(i + 1) != '/');
		i = fileAsString.find('/', i + 2);
	}
}

File File::getFolder() const {
	size_t i = fileAsString.find_last_of('/');
	if(i == string::npos) {
		return File(true, string("./", 2));
	}
	return File(true, fileAsString.substr(0, i + 1));
}

string File::getFilename() const {
	size_t i = fileAsString.find_last_of('/');
	if(i == string::npos) {
		return fileAsString;
	}
	return fileAsString.substr(i + 1);
}

bool File::mayBeFolder() const {
	if(fileAsString.empty()) {
		return false;
	}
	return fileAsString.at(fileAsString.length() - 1) == '/';
}

bool File::isAbsolutePath() const {
	if(fileAsString.empty()) {
		return false;
	}
	return fileAsString.at(0) == '/';
}

void File::makeLikeFolder() {
	if(fileAsString.empty()) {
		fileAsString.assign("./", 2);
	}
	if(fileAsString.at(fileAsString.length() - 1) != '/') {
		fileAsString.append("/", 1);
	}
}

bool File::exists(const string &path) {
	static struct stat stat; // Can use static here as this is never read, therefor no race conditions or similar
	if(lstat(path.c_str(), &stat) != 0) {
		return false;
	}
	return true;
}

bool File::isLink(const string &path) {
	struct stat st;
	if(lstat(path.c_str(), &st) != 0) {
		return false;
	}
	return (S_ISLNK(st.st_mode));
}

bool File::isReg(const string &path) {
	struct stat st;
	if(lstat(path.c_str(), &st) != 0) {
		return false;
	}
	return (S_ISREG(st.st_mode));
}

bool File::isDir(const string &path) {
	struct stat st;
	if(stat(path.c_str(), &st) != 0) {
		return false;
	}
	return (S_ISDIR(st.st_mode));
}

void File::rename(const string &oldpath, const string &newpath) {
	if(::rename(oldpath.c_str(), newpath.c_str()) == 0) {
		LOGDISK("mv \"" << oldpath << "\" \"" << newpath << "\"");
		return;
	}
	const int e = errno;
	switch(e) {
		case EACCES: throw runtime_error(string("EACCES: rename: ") + oldpath + ", " + newpath);
		case EBUSY: throw runtime_error(string("EBUSY: rename: ") + oldpath + ", " + newpath);
		case EFAULT: throw runtime_error(string("EFAULT: rename: ") + oldpath + ", " + newpath);
		case EINVAL: throw runtime_error(string("EINVAL: rename: ") + oldpath + ", " + newpath);
		case EISDIR: throw runtime_error(string("EISDIR: rename: ") + oldpath + ", " + newpath);
		case ELOOP: throw runtime_error(string("ELOOP: rename: ") + oldpath + ", " + newpath);
		case EMLINK: throw runtime_error(string("EMLINK: rename: ") + oldpath + ", " + newpath);
		case ENAMETOOLONG: throw runtime_error(string("ENAMETOOLONG: rename: ") + oldpath + ", " + newpath);
		case ENOSPC: throw runtime_error(string("ENOSPC: rename: ") + oldpath + ", " + newpath);
		case EEXIST: throw runtime_error(string("EEXIST: rename: ") + oldpath + ", " + newpath);
		case ENOTEMPTY: throw runtime_error(string("ENOTEMPTY: rename: ") + oldpath + ", " + newpath);
		case EPERM: throw runtime_error(string("EPERM: rename: ") + oldpath + ", " + newpath);
		case EROFS: throw runtime_error(string("EROFS: rename: ") + oldpath + ", " + newpath);
		case EXDEV: throw runtime_error(string("EXDEV: rename: ") + oldpath + ", " + newpath);
		case ENOENT: throw runtime_error(string("ENOENT: rename: ") + oldpath + ", " + newpath);
		case ENOMEM: throw runtime_error(string("ENOENT: rename: ") + oldpath + ", " + newpath);
		case ENOTDIR: throw runtime_error(string("ENOTDIR: rename: ") + oldpath + ", " + newpath);
		default: {
			ostringstream buffer;
			buffer << "Error " << e << ": rename: " << oldpath << ", " << newpath;
			throw runtime_error(buffer.str());
		}
	}
}

void File::readlink(const string &path, string &link) {
	char rbuf[1000 + 1];
	ssize_t a = ::readlink(path.c_str(), rbuf, 1000);
	if(a != -1) {
		assert(a < 1000);
		if(a == 0) {
			throw runtime_error(string("Invalid empty link content: ") + path);
		}
		link.assign(rbuf, a);
		if(link.at(0) == '/') {
			return;
		}
		return;
	}
	const int e = errno;
	switch(e) {
		case EACCES: throw runtime_error(string("EACCES: readlink: ") + path);
		case EFAULT: throw runtime_error(string("EFAULT: readlink: ") + path);
		case EINVAL: throw runtime_error(string("EINVAL: readlink: ") + path);
		case EIO: throw runtime_error(string("EIO: readlink: ") + path);
		case ENAMETOOLONG: throw runtime_error(string("ENAMETOOLONG: readlink: ") + path);
		case ENOENT: throw runtime_error(string("ENOENT: readlink: ") + path);
		case ENOMEM: throw runtime_error(string("ENOENT: readlink: ") + path);
		case ENOTDIR: throw runtime_error(string("ENOTDIR: readlink: ") + path);
		default: {
			ostringstream buffer;
			buffer << "Error " << e << ": readlink: " << path;
			throw runtime_error(buffer.str());
		}
	}
}

string File::readlink(const string &path) {
	string result;
	readlink(path, result);
	return result;
}

void File::mkdir(const string &path) {
	if(isDir(path)) {
		return;
	}
	char buffer[1000 + 1];
	const size_t l = path.length();
	assert(l <= 1000);
	memcpy(buffer, path.c_str(), l);
	buffer[l] = '\0';

	size_t i = 1;
	while((i = path.find('/', i)) != string::npos) {
		buffer[i] = '\0';
		assureIsDir(buffer);
		buffer[i] = '/';
		i = path.find_first_not_of('/', i);
	}
	if(i != path.length()) {
		assureIsDir(buffer);
	}
}

void File::mkdirForFile(const string &path) {
	char buffer[1000 + 1];
	const size_t l = path.length();
	assert(l <= 1000);
	memcpy(buffer, path.c_str(), l);
	buffer[l] = '\0';
	assert(buffer[l - 1] != '/');

	size_t i = 1;
	while((i = path.find('/', i)) != string::npos) {
		buffer[i] = '\0';
		assureIsDir(buffer);
		buffer[i] = '/';
		i = path.find_first_not_of('/', i);
	}
	assert(i + 1 < path.length());
}

void File::rmdir(const string &path) {
	struct stat st;
	if(stat(path.c_str(), &st) != 0) {
		return;
	}
	if(::rmdir(path.c_str()) == 0) {
		LOGDISK("rmdir \"" << path << "\"");
		return;
	}
	throw SyscallException(errno, "rmdir", path);
}

void File::unlink(const string &path) {
	struct stat st;
	if(lstat(path.c_str(), &st) != 0) {
		return;
	}
	if(::unlink(path.c_str()) == 0) {
		LOGDISK("rm \"" << path << "\"");
		return;
	}
	throw SyscallException(errno, "unlink", path);
}

void File::assureIsDir(const char *path) {
	assureIsDir(path, 0755);
}

void File::assureIsDir(const char *path, int mode) {
	struct stat st;
	if(::stat(path, &st) == 0) {
		if(!S_ISDIR(st.st_mode)) {
			throw runtime_error(string("path exists but is not a folder: ") + path);
		}
		return;
	}
	if(::mkdir(path, mode) == 0) {
		LOGDISK("mkdir \"" << path << "\"");
		return;
	}
	throw SyscallException(errno, "mkdir", path);
}

void File::link(const string &target, const string &file) {
	if(::symlink(target.c_str(), file.c_str()) == 0) {
		LOGDISK("ln -s \"" << target << "\", \"" << file << "\"");
		return;
	}
	throw SyscallException(errno, "symlink", target, file);
}

File::dir_iterator &File::dir_iterator::open(const std::string &p) {
	assert(!p.empty());
	dir = opendir(p.c_str());
	if(dir == NULL) {
		throw SyscallException(errno, "opendir", p);
	}
	path = p;
	if(path[path.length() - 1] != '/') { path.append("/", 1); }
	++(*this);
	return *this;
}

File::dir_iterator::dir_iterator()
: dir(NULL)
, path()
, file()
{
}

File::dir_iterator::~dir_iterator() {
	if(dir != NULL) {
		closedir(dir);
		dir = NULL;
	}
}

File::dir_iterator::dir_iterator(const dir_iterator &other)
: dir(other.dir)
, path(other.path)
, file(other.file)
{
	const_cast<dir_iterator&>(other).dir = NULL;
}

File::dir_iterator &File::dir_iterator::operator=(const dir_iterator &other) {
	dir = other.dir;
	path = other.path;
	file = other.file;
	const_cast<dir_iterator&>(other).dir = NULL;
	return *this;
}

File::dir_iterator &File::dir_iterator::operator++() {
	if(dir == NULL) {
		return *this;
	}
	struct dirent *ent = readdir(dir);
	if(ent == NULL) {
		closedir(dir);
		dir = NULL;
		path = "";
		file = "";
		return *this;
	}
	file = ent->d_name;
	if(file == ".") {
		return ++(*this);
	}
	if(file == "..") {
		return ++(*this);
	}
	d_type = ent->d_type;
	return *this;
}

bool File::dir_iterator::isHidden() const {
	return file.at(0) == '.';
}

bool File::dir_iterator::isDir() const {
	return File::isDir(path + file);
}

bool File::dir_iterator::isReg() const {
//	return (d_type == DT_REG);
	return File::isReg(path + file);
}

bool File::dir_iterator::isLink() const {
//	return (d_type == DT_LNK);
	return File::isLink(path + file);
}

bool File::dir_iterator::operator!=(const dir_iterator &other) {
	return (path != other.path) || (file != other.file);
}

File::dir_iterator File::begin(const string &path) {
	dir_iterator a;
	return a.open(path);
}

File::dir_iterator File::end(const string &path) {
	return dir_iterator();
}

std::string File::dir_iterator::readlink() const {
	return File(*this).readlink();
}

