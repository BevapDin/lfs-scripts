#pragma once

#include <string>

/**
 * Mapper for mmap-system-call, in other words memory mapping of a file.
 * Some restrictions: see "man 2 mmap" (only mapping of files, mapping
 * is readonly, file chould not change while mapped).
 * Usage:
 * Map a file, print it on stdout:
 * <code>MMap maped("myfile"); fwrite(maped.map(), 1, maped.len(), std::cout);</code>
 * More detailed:
 * <code>
 * MMap maped("myfile"); // Create object
 * // or reuse some:
 * maped.setFilename("myotherfile"); // existing maping is automaticly closed
 * // Create the mapping:
 * maped.open();
 * // retrive length and address of mapping:
 * const void *mappedData = maped.map();
 * const size_t mappedLength = mapped.len();
 * // Note: mapped.map() will automaticly create the mapping.
 * // End mapping (close file, remove mapping):
 * maped.close();
 * // Or simply destroy the object.
 * </code>
 * Exceptions:
 * open() and len() automaticly access the filesystem if their
 * return value is not jet determined and may therefor throw an error.
 * But a second call to them is safe, if the first call was successful
 * and nothing in the MMap object has changed:
 * <code>
 * maped.open(); // may throw
 * const void *ptr = maped.map(); // won't throw, because previous open has not thrown
 * </code>
 * open() is just a wrapped to map().
 */
class MMap {
protected:
	std::string _filename;
	size_t _length;
	void *_addr;
	int _fd;

public:
	MMap();
	MMap(const std::string &filename);
	~MMap() throw();

	void close();
	const std::string &getFilename() const;
	void setFilename(const std::string &filename);
	size_t len();
	void open();
	const void *map();

	template<typename T>
	const T *pointerTo() {
		return reinterpret_cast<const T*>(map());
	}
	template<typename T>
	size_t countOf() {
		return len() / sizeof(T);
	}
};
