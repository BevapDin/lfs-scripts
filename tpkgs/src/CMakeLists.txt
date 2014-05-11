cmake_minimum_required(VERSION 2.8)

include_directories(.)

set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(POPT REQUIRED)
include_directories(${POPT_INCLUDE_DIRS})

add_executable(tpkgs
	main.cpp
	File.cpp
	MMap.cpp
	Package.cpp
	TPKGS.cpp
	InstallItem.cpp
	Version.cpp
	MXOSet.cpp
	Config.cpp
	FindFilesOnInstallDevice.cpp
	FindFilesInMappedDirs.cpp
	FindFilesInInstallDirs.cpp
	FindFiles.cpp
	Popt.cpp
	DiffVersions.cpp
)

set_target_properties(tpkgs PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(tpkgs PROPERTIES LINK_SEARCH_END_STATIC 1)

target_link_libraries(tpkgs ${Boost_LIBRARIES})
target_link_libraries(tpkgs ${POPT_LIBRARY})


