#########################################################################
#  BBM CMake Build Environment     
#########################################################################
#
# Specify the config file with -DBBM_CONFIG=<file>. If none specified try
# 'config.cmake'. If not found either, set default values.
#
# Specify compiler presets with --preset (See CMakePresets.json for
# available presets). E.g., cmake --preset=gcc
#
# set BBM_FORCE_PYTHON_VERSION to force a python version otherwise
# the newest is used. For example:
#
# cmake --preset=gcc -DBBM_FORCE_PYTHON_VERSION=3.6
#
#########################################################################


#########################################################################
# Initialization
#########################################################################
cmake_minimum_required(VERSION 3.19)
project(he)

#########################################################################
# Setup BBM
#########################################################################
find_package(BBM REQUIRED)

#########################################################################
# Get source files
#########################################################################
file(GLOB SRC src/*cpp)
file(GLOB EXECLIST bin/*cpp)

########################################################################
# Set Compile Targets
########################################################################
foreach(EXEC ${EXECLIST})
  get_filename_component(EXECNAME ${EXEC} NAME_WE)
  add_executable(${EXECNAME} ${EXEC} ${SRC})

  target_compile_definitions(${EXECNAME} INTERFACE bbm::core)
  target_include_directories(${EXECNAME} PRIVATE bbm::core)
  target_link_libraries(${EXECNAME} PRIVATE bbm::core)

endforeach()
  
