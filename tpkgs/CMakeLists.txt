cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0 -static" CACHE STRING "" FORCE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Allways make a static build
SET(CMAKE_CXX_FLAGS "-static" CACHE STRING "" FORCE)
#set(CMAKE_CXX_FLAGS "-static-libgcc -static-libstdc++" CACHE STRING "" FORCE)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

if(MSVC)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	# Update if necessary
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

project(tpkgs)

if(CMAKE_COMPILER_IS_GNUCXX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wno-sign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused -Wall -Wextra -Wno-old-style-cast -Woverloaded-virtual -g -ggdb -O0")
endif()

include_directories(.)

add_subdirectory(src)
