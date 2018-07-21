# Include dirs.
include_directories(.)
include_directories(${metasim_INCLUDE_DIRS})


set(SOURCE_FILES
  AVRTask.cpp
  bwi.cpp
  capacitytimer.cpp
  cbserver.cpp
  cpu.cpp
  edfsched.cpp
  exeinstr.cpp
  fcfsresmanager.cpp
# feedbackarsim.cpp
  feedback.cpp
  feedbacktest.cpp
  fifosched.cpp
  fpsched.cpp
  grubserver.cpp
  instr.cpp
  interrupt.cpp
  json_trace.cpp
  jtrace.cpp
  kernel.cpp
  kernevt.cpp
  load.cpp
  mrtkernel.cpp
  piresman.cpp
  pollingserver.cpp
  powermodel.cpp
  reginstr.cpp
  regsched.cpp
  regtask.cpp
  resmanager.cpp
  resource.cpp
  rmsched.cpp
  rrsched.cpp
# rtload.cpp
  rttask.cpp
  schedinstr.cpp
  schedpoints.cpp
  schedrta.cpp
  scheduler.cpp
  server.cpp
  sparepot.cpp
  sporadicserver.cpp
  supercbs.cpp
  suspend_instr.cpp
  task.cpp
  taskevt.cpp
  texttrace.cpp
  threinstr.cpp
  timer.cpp
  traceevent.cpp
  tracepower.cpp
  waitinstr.cpp
)

set(HEADER_FILES
  abskernel.hpp
  absresmanager.hpp
  abstask.hpp
  AVRTask.hpp
  bwi.hpp
  capacitytimer.hpp
  cbserver.hpp
  cpu.hpp
  edfsched.hpp
  exeinstr.hpp
  fcfsresmanager.hpp
# feedbackarsim.hpp
  feedback.hpp
  feedbacktest.hpp
  fifosched.hpp
  fpsched.hpp
  grubserver.hpp
  instr.hpp
  interrupt.hpp
  json_trace.hpp
  jtrace.hpp
  kernel.hpp
  kernevt.hpp
  load.hpp
  mrtkernel.hpp
  piresman.hpp
  pollingserver.hpp
  powermodel.hpp
  reginstr.hpp
  resmanager.hpp
  resource.hpp
  rmsched.hpp
  rrsched.hpp
# rtload.hpp
  rttask.hpp
  schedinstr.hpp
  schedpoints.hpp
  schedrta.hpp
  scheduler.hpp
  server.hpp
  sparepot.hpp
  sporadicserver.hpp
  supercbs.hpp
  supervisor.hpp
  suspend_instr.hpp
  taskevt.hpp
  taskexc.hpp
  task.hpp
  taskstat.hpp
  texttrace.hpp
  threinstr.hpp
  timer.hpp
  traceevent.hpp
  tracepower.hpp
  waitinstr.hpp
)

# Create a library called "rtlib" which includes the source files.
add_library(${PROJECT_NAME} ${LIBRARY_TYPE} ${SOURCE_FILES})

target_link_libraries (${PROJECT_NAME} ${metasim_LIBRARIES})

target_compile_features (${PROJECT_NAME} PRIVATE cxx_range_for)

set_property(TARGET ${PROJECT_NAME} PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             ${PROJECT_SOURCE_DIR}/src)

# Export.
export (TARGETS ${PROJECT_NAME} FILE "./rtlibConfig.cmake")
export (PACKAGE ${PROJECT_NAME})
