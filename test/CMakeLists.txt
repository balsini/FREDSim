# Include dirs.
include_directories(.)
include_directories(../src)
include_directories(${metasim_INCLUDE_DIRS})

# Environment-based settings.
if(APPLE)
	set(LIB_TYPE "SHARED")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -std=c++0x")
	
	if(EXISTS "${metasim_DIR}/libmetasim.dylib")
		set(metasim_LIBRARY ${CMAKE_LIBRARY_PATH} "${metasim_DIR}/libmetasim.dylib")
	elseif(EXISTS "${metasim_DIR}/Debug/libmetasim.dylib")
		set(metasim_LIBRARY ${CMAKE_LIBRARY_PATH} "${metasim_DIR}/Debug/libmetasim.dylib")
	elseif(EXISTS "${metasim_DIR}/Release/libmetasim.dylib")
		set(metasim_LIBRARY ${CMAKE_LIBRARY_PATH} "${metasim_DIR}/Release/libmetasim.dylib")
	endif()
	
elseif(UNIX)
	set(LIB_TYPE "SHARED")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -std=c++0x")
	set(metasim_LIBRARY "${metasim_DIR}/libmetasim.so")
	
elseif(WIN32)	
	if(EXISTS "${metasim_DIR}/Debug/metasim.lib")
		set(metasim_LIBRARY "${metasim_DIR}/Debug/metasim.lib")
	elseif(EXISTS "${metasim_DIR}/Release/metasim.lib")
		set(metasim_LIBRARY "${metasim_DIR}/Release/metasim.lib")
	endif()
		
endif()

# Create the executable.
add_executable(runtest test_main.cpp cbs.cpp test_task.cpp test_mrt.cpp test_AVR.cpp test_accelerated.cpp)

# Indicate that rtlib need rtlib library.
target_link_libraries(runtest rtlib ${metasim_LIBRARY})
