cmake_minimum_required (VERSION 2.6)
project (rtlib)

# Packages.
find_package(metasim REQUIRED)

# set metasim include dir
set(metasim_INCLUDE_DIRS "${metasim_DIR}/../../src")

# Include dirs.
add_subdirectory (src)
add_subdirectory (examples)
#add_subdirectory (test)

# Export.
export(TARGETS rtlib FILE "./rtlibConfig.cmake")
export(PACKAGE rtlib)
