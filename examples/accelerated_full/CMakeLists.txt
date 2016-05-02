# Include Environment-based settings.
include(${CMAKE_CURRENT_DOURCE_DIR}../common_settings.txt)

# Create the executable.
add_executable(accelerated_full accelerated_full.cpp generator.cpp builder.cpp rta_ss.cpp delay_bound.cpp tinyxml/tinyxml2.cpp)
add_executable(standalone standalone.cpp generator.cpp)

# Indicate that the executable needs metasim and rtlib library.
target_link_libraries( accelerated_full boost_system boost_filesystem emon glpk rtlib ${metasim_LIBRARY})
