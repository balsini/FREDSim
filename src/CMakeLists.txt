# Include dirs.
include_directories(.)
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
	set(LIB_TYPE "STATIC")
	if(EXISTS "${metasim_DIR}/Debug/metasim.lib")
		set(metasim_LIBRARY "${metasim_DIR}/Debug/metasim.lib")
	elseif(EXISTS "${metasim_DIR}/Release/metasim.lib")
		set(metasim_LIBRARY "${metasim_DIR}/Release/metasim.lib")
	endif()
endif()

# Create a library called "rtlib" which includes the source files.
add_library(rtlib ${LIB_TYPE} capacitytimer.cpp cbserver.cpp cpu.cpp 
  edfsched.cpp exeinstr.cpp fcfsresmanager.cpp feedback.cpp feedbacktest.cpp 
  fifosched.cpp fpsched.cpp grubserver.cpp interrupt.cpp jtrace.cpp 
  kernel.cpp kernevt.cpp load.cpp mrtkernel.cpp piresman.cpp pollingserver.cpp 
  reginstr.cpp regsched.cpp regtask.cpp resmanager.cpp resource.cpp 
  rmsched.cpp rrsched.cpp rttask.cpp acceleratedtask.cpp schedinstr.cpp schedpoints.cpp schedrta.cpp 
  scheduler.cpp server.cpp sparepot.cpp sporadicserver.cpp supercbs.cpp 
  task.cpp taskevt.cpp texttrace.cpp threinstr.cpp timer.cpp traceevent.cpp 
  tracepower.cpp waitinstr.cpp instr.cpp suspend_instr.cpp AVRTask.cpp json_trace.cpp ps_trace.cpp)

# Indicate that rtlib need metasim library.
target_link_libraries( rtlib  ${metasim_LIBRARY} )
