#pragma once

#include <string>
#include <unistd.h>
#include <dirent.h>
#include <boost/filesystem/path.hpp>

#define LOGDISK(text) do { if(File::getLogDiskAccess()) { std::cout << text << "\n"; } } while(false)

class SyscallException : public std::exception {
protected:
	int errNumber;
	const char *syscall;
	const std::string path1;
	const std::string path2;
	mutable std::string message;

public:
	SyscallException(int errNumber, const char *syscall, const std::string &path1, const std::string &path2);
	SyscallException(int errNumber, const char *syscall, const std::string &path1);
	virtual ~SyscallException() throw();
	virtual const char* what() const throw();
	void formatMessage() const;
};

class File {
protected:
	static bool logDiskAccess;
	std::string fileAsString;

	File(bool isClear, const std::string &other);

	void cheapNormalize();
	static void assureIsDir(const char *path);
	static void assureIsDir(const char *path, int mode);

public:
	File();
	File(const File &other);
	File(const std::string &other);
	File(const char *other);
	File(const boost::filesystem::path &other) : File(other.string()) { }
	File(const File &absolutePath, const std::string &relativeFile);
	~File();

	File &operator=(const File &other);
	File &operator=(const std::string &other);
	bool operator==(const File &other) const;
	bool operator==(const std::string &other) const;
	bool operator<(const File &other) const;
	bool operator<(const std::string &other) const;

	/**
	 * Checks if file path ends with '/',
	 * no actual file system access is done.
	 */
	bool mayBeFolder() const;
	/**
	 * Appends a '/' if the last char is not
	 * jet a '/'.
	 */
	void makeLikeFolder();
	bool isAbsolutePath() const;

	File getFolder() const;
	std::string getFilename() const;

	File &append(const char *str);
	File &append(const std::string &str);

	inline operator const std::string &() const { return asString(); };
	inline const std::string &asString() const { return fileAsString; };

	static inline void setLogDiskAccess(bool logDiskAccess) { File::logDiskAccess = logDiskAccess; };
	static inline bool getLogDiskAccess() { return File::logDiskAccess; };

	/**
	 * Returns true if the object exits, even if it's
	 * a broken link. Like if `ls path` succeeds than
	 * this function returns true.
	 */
	static bool exists(const std::string &path);
	/**
	 * Returns true if this is a link, does not
	 * check the target of the link, so it may be a
	 * broken link.
	 */
	static bool isLink(const std::string &path);
	/**
	 * Returns true if this is _not_ a link _and_
	 * it's a regular file.
	 */
	static bool isReg(const std::string &path);
	/**
	 * Returns true if this is a folder or if
	 * it's a (non-broken) link to a folder.
	 * (on broken links it returns false).
	 */
	static bool isDir(const std::string &path);

	static void rename(const std::string &oldpath, const std::string &newpath);
	static void readlink(const std::string &path, std::string &link);
	static std::string readlink(const std::string &path);
	static void mkdir(const std::string &path);
	static void mkdirForFile(const std::string &path);
	static void rmdir(const std::string &path);
	static void link(const std::string &target, const std::string &file);
	static void unlink(const std::string &path);

	inline bool exists() const { return File::exists(fileAsString); };
	inline bool isLink() const { return File::isLink(fileAsString); };
	inline bool isReg() const { return File::isReg(fileAsString); };
	inline bool isDir() const { return File::isDir(fileAsString); };

	inline void rename(const File &newPath) const { File::rename(fileAsString, newPath.fileAsString); };
	inline void rename(const std::string &newPath) const { File::rename(fileAsString, newPath); };
	inline std::string readlink() const { return readlink(fileAsString); };
	inline void mkdir() const { File::mkdir(fileAsString); };
	inline void mkdirForFile() const { File::mkdirForFile(fileAsString); };
	inline void rmdir() const { File::rmdir(fileAsString); };
	inline void unlink() const { File::unlink(fileAsString); };

	inline void link(const File &target) const { File::link(target.fileAsString, fileAsString); };
	inline void link(const std::string &target) const { File::link(target, fileAsString); };

	class dir_iterator {
	protected:
		DIR *dir;
		std::string path;
		std::string file;
		int d_type;
		friend class File;
		dir_iterator &open(const std::string &path);
		dir_iterator();
	private:
	public:
		dir_iterator(const dir_iterator &other);
		dir_iterator &operator=(const dir_iterator &other);
		~dir_iterator();
		dir_iterator &operator++();
		bool operator!=(const dir_iterator &other);
		inline const std::string &getPath() const { return path; };
		inline const std::string &getFile() const { return file; };
		inline const std::string &operator*() const { return file; };
		inline const std::string *operator->() const { return &file; };
		bool isHidden() const;
		bool isDir() const;
		bool isReg() const;
		bool isLink() const;
		std::string readlink() const;
	};

	File(const dir_iterator &it);

	static dir_iterator begin(const std::string &path);
	static dir_iterator end(const std::string &path);

	inline dir_iterator begin() const { return begin(fileAsString); }
	inline dir_iterator end() const { return end(fileAsString); }
};

inline std::string operator+(const std::string &a, const File &b) { return a + b.asString(); }
//inline std::string operator+(const File &a, const std::string &b) { return File(a).append(b); }
inline std::ostream &operator<<(std::ostream &stream, const File &file) { return (stream << file.asString()); }
