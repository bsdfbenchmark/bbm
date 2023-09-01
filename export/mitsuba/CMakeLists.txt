#########################################################################
# Mitsuba 0.5 BBM BRDF plugin    
#########################################################################
#
# The default behavior can be overwritten by setting the following CMake
# variables:
#
# MITSUBA_EXPORTER: toggle compilation ON/OFF, default = ON. Note this flag is
#                   secondary to the BBM_EXPORTERS master flag.
# MITSUBA_PATH: location of mitsuba coude. By default search one level below
#               the BBM source dir.
# MITSUBA_BINARY_PATH: location of mitsuba executavbles. By default search all
#                      subdirs of MITSUBA_PATH
# MITSUBA_COMPILER_OPTIONS: add compiler options. By default it uses the same
#                           options as for the BBM library.
#
#########################################################################


#########################################################################
# Initialization 
#########################################################################
cmake_minimum_required(VERSION 3.15)
project(bbm_mitsuba_plugin)

if(NOT BBM_SOURCE_DIR)
  message(FATAL_ERROR "Expected to be called from BBM's CMake script.")
  return()
endif()

#########################################################################
# Helper Scripts
#########################################################################
list(APPEND CMAKE_MODULE_PATH ${bbm_mitsuba_plugin_SOURCE_DIR}/cmake ${bbm_SOURCE_DIR}/cmake )
include(BBM_Mitsuba)

#########################################################################
# Configure
#########################################################################
get_filename_component(BBM_PARENT_DIR ${BBM_SOURCE_DIR} DIRECTORY) 
option(MITSUBA_PATH "Path to the Mitsuba source" OFF)
option(MITSUBA_BINARY_PATH "Path to Mistuba executables" OFF)
option(MITSUBA_EXPORTER "Compile the Mitsuba Exporter" ON)

if(NOT MITSUBA_PATH)
  set(MITSUBA_PATH ${BBM_PARENT_DIR}/mitsuba)
endif()

if(NOT MITSUBA_EXPORTER)
  message(STATUS "Skipping Mitsuba Exporter (MITSUBA_EXPORTER=OFF)")
endif()

#########################################################################
# C++ Options 
#########################################################################
# Enable C++20
set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ version selection")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#########################################################################
# Setup Mitsuba Compile 
#########################################################################

# Force Mitsuba build type to release
set(CMAKE_BUILD_TYPE "Release")

#########################################################################
# Check Mitsuba Install 
#########################################################################
getMitsubaPath()
getMitsubaCompileSettings()
link_directories(mitsuba_bbm ${MITSUBA_BINARY_PATH})

#########################################################################
# Get BBM properties 
#########################################################################
get_target_property(include_dirs ${BBM_NAME} INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(compile_def ${BBM_NAME} INTERFACE_COMPILE_DEFINITIONS)
get_target_property(link_libs ${BBM_NAME} INTERFACE_LINK_LIBRARIES)

#########################################################################
# Get CPP files
#########################################################################
file(GLOB SRC src/*cpp)

#########################################################################
# Set Compile Targets 
#########################################################################
bsdf_plugin(mitsuba_bbm ${SRC})
target_compile_definitions(mitsuba_bbm PUBLIC ${compile_def})
target_include_directories(mitsuba_bbm SYSTEM PUBLIC ${MITSUBA_PATH}/include)
target_include_directories(mitsuba_bbm PUBLIC ${bbm_mitsuba_plugin_SOURCE_DIR}/include ${include_dirs})
target_link_libraries(mitsuba_bbm ${link_libs})

#########################
# Copy Files to Mistuba #
#########################
add_custom_command(TARGET mitsuba_bbm POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:mitsuba_bbm>" "${MITSUBA_BINARY_PATH}/plugins/$<TARGET_FILE_NAME:mitsuba_bbm>" COMMENT "Copying to Mitsuba plugin directory"
  )
