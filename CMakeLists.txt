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
cmake_minimum_required(VERSION 3.21)
project(BBM)
set(BBM_VERSION "0.5.1")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

#########################################################################
# Helper methods
#########################################################################
include("${BBM_SOURCE_DIR}/cmake/bbm_helpers.cmake")

#########################################################################
# Configure 
#########################################################################
# set standard values
get_filename_component(BBM_NAME ${BBM_SOURCE_DIR} NAME)
option(BBM_EXPORTERS "Compile Exporters" OFF)
option(BBM_PYTHON "Include BBM python interface" OFF)
option(BBM_BINARIES "Include BBM binaries" OFF)
option(BBM_PYTHON_LIBRARIES "Include BBM python libraries" OFF)
option(BBM_BSDF_FORWARD "Enable BSDF Function forwarding" ON)

# Load config-file if specified, otherwise try 'config.cmake'.
if(BBM_CONFIG)
  if(NOT EXISTS "${BBM_CONFIG}")
    message(STATUS "WARNING: configuration '${BBM_CONFIG}' not found. Trying default configuration.")
  else()
    include(${BBM_CONFIG})
    message(STATUS "Configuration: '${BBM_CONFIG}'")
  endif()
endif()

if(NOT BBM_CONFIG)
  if(NOT EXISTS "${CMAKE_SOURCE_DIR}/config.cmake")
    message(STATUS "Configuration: DEFAULT.")
  else()
    include("${CMAKE_SOURCE_DIR}/config.cmake")
    message(STATUS "Confguration: 'config.cmake'")
  endif()
endif()


# determine build type:
# 1) use build type if specified by the user.
# 2) otherwise use default build type.
if(CMAKE_BUILD_TYPE)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" TYPE)
  message(STATUS "Using user specified build type: ${TYPE}")
else()
  set(CMAKE_BUILD_TYPE DEBUG)
  message(STATUS "Using default build type: ${CMAKE_BUILD_TYPE}")
endif()

#########################################################################
# Create Interface Target with ${BBM_NAME}
#########################################################################
add_library(${BBM_NAME} INTERFACE)

#########################################################################
# Pass BBM_NAME, BBM_BRDF, BBM_BSDF_FORWARD, BBM_BSDF_DEFAULT_IMPORTER
#########################################################################
target_compile_definitions(${BBM_NAME} INTERFACE BBM_VERSION=${BBM_VERSION})
target_compile_definitions(${BBM_NAME} INTERFACE BBM_NAME=${BBM_NAME})

message(STATUS "BBM_VERSION; ${BBM_VERSION}")
message(STATUS "BBM_NAME set to: ${BBM_NAME}")

if(BBM_BSDF_FORWARD)
  target_compile_definitions(${BBM_NAME} INTERFACE BBM_BSDF_ENABLE_FORWARD)
  message(STATUS "BBM_BSDF_FORWARD: enabled")
else()
  message(STATUS "BBM_BSDF_FORWARD: disabled")
endif()

if(NOT BBM_DEFAULT_BSDF_IMPORTER)
  set(BBM_DEFAULT_BSDF_IMPORTER "string")
endif()
if("${BBM_DEFAULT_BSDF_IMPORTER}" STREQUAL "string")
  target_compile_definitions(${BBM_NAME} INTERFACE BBM_STRING_BSDF_IMPORTER)
  message(STATUS "Default BSDF importer: BBM_STRING_BSDF_IMPORTER")
elseif("${BBM_DEFAULT_BSDF_IMPORTER}" STREQUAL "python" AND BBM_PYTHON AND BBM_EMBED_CONFIG)
  target_compile_definitions(${BBM_NAME} INTERFACE BBM_PYTHON_BSDF_IMPORTER BBM_CONFIG=${BBM_EMBED_CONFIG})
  message(STATUS "Default BSDF importer: BBM_PYTHON_BSDF_IMPORTER (EMBEDDED)")
elseif("${BBM_DEFAULT_BSDF_IMPORTER}" STREQUAL "python" AND BBM_PYTHON)
  target_compile_definitions(${BBM_NAME} INTERFACE BBM_PYTHON_BSDF_IMPORTER)
  message(STATUS "Default BSDF importer: BBM_PYTHON_BSDF_IMPORTER")
else()
  target_compile_definitions(${BBM_NAME} INTERFACE BBM_STRING_BSDF_IMPORTER)
  message(STATUS "No valid BBM_DEFAULT_BSDF_IMPORTER selected; using BBM_STRING_BSDF_IMPORTER")
endif()

# propagate BBM_NAME to parent scope.
if(NOT PROJECT_IS_TOP_LEVEL)
  set(BBM_NAME ${BBM_NAME} PARENT_SCOPE)
endif()

#########################################################################
# Enable C++20
#########################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#########################################################################
# Import Dependencies 
#########################################################################
message(STATUS "Processing Required External Dependencies:")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

# pyding11
set(BBM_FORCE_PYTHON_VERSION "" CACHE STRING "Force Python version.")

if((BBM_PYTHON) AND (NOT PYBIND11_PATH))
  message(STATUS "PYBIND11_PATH not set; using default path.")
  set(PYBIND11_PATH ${BBM_SOURCE_DIR}/ext/pybind11)
endif()

if((BBM_PYTHON) AND (NOT IS_DIRECTORY "${PYBIND11_PATH}"))
    set(BBM_PYTHON OFF)
    message(STATUS "Pybind11 not found in ${PYBIND11_PATH}. Disabling BBM_PYTHON.")
endif()

if(BBM_PYTHON)
    list(APPEND CMAKE_MODULE_PATH "${PYBIND11_PATH}/tools")
    find_package(PythonLibsNew ${BBM_FORCE_PYTHON_VERSION} REQUIRED)

    set(PYBIND11_INCLUDE_DIRS ${PYBIND11_PATH}/include ${PYTHON_INCLUDE_DIR})
    set(PYBIND11_PYTHON_VERSION ${PYTHON_VERSION})

    # add pybind
    add_subdirectory(${PYBIND11_PATH})
endif()

# Final check
if(NOT ${PYTHONLIBS_FOUND})
    set(BBM_PYTHON OFF)
    message(STATUS "No python libraries found. Disabling BBM_PYTHON.")
endif()

# set compile targets
if(${BBM_PYTHON})
    # set compile options
    message(STATUS "Using Python ${PYBIND11_PYTHON_VERSION}")
    target_compile_definitions(${BBM_NAME} INTERFACE BBM_PYTHON)
    target_compile_definitions(${BBM_NAME} INTERFACE BBM_PYTHON_LIBRARY=${PYTHON_LIBRARY})
    target_compile_definitions(${BBM_NAME} INTERFACE BBM_PYTHON_VERSION=${PYBIND11_PYTHON_VERSION})
    target_link_libraries(${BBM_NAME} INTERFACE pybind11::embed)
endif()

# dynamic link libs
target_link_libraries(${BBM_NAME} INTERFACE ${CMAKE_DL_LIBS})

list(POP_BACK CMAKE_MESSAGE_INDENT)


#########################################################################
# Setup BBM Backbone
#########################################################################
message(STATUS "Importing Backbone: ${BBM_BACKBONE}")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

setup_backbone()

if(BBM_BACKBONE_CONFIGURATIONS)
  message(STATUS "Available configurations: ${BBM_BACKBONE_CONFIGURATIONS}")
elseif(BBM_PYTHON_CONFIG)
  message(STATUS "No backbone configurations available (BBM_BACKBONE_CONFIGURATIONS is empty). Disabling stand-alone python libraries.")
  set(BBM_PYTHON_CONFIG OFF)
endif()

target_compile_definitions(${BBM_NAME} INTERFACE BBM_BACKBONE=${BBM_BACKBONE})

list(POP_BACK CMAKE_MESSAGE_INDENT)

#########################################################################
# Generate header files
#########################################################################
message(STATUS "Autogenerating Header Files:")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

# set directories to scan for BSDF models
if(NOT BBM_BSDFMODELS)
  set(BBM_BSDFMODELS "bsdfmodel/*h" "staticmodel/*h")
endif()

# generate header
write_bsdfmodel_header("${BBM_SOURCE_DIR}/include" "${BBM_BSDFMODELS}")

list(POP_BACK CMAKE_MESSAGE_INDENT)


#########################################################################
# Scan directories for files
#########################################################################
# Set include directories
target_include_directories(${BBM_NAME} INTERFACE ${BBM_SOURCE_DIR}/include)

# Get Python files
set(PYSRC "")
if(BBM_PYTHON)
  file(GLOB PYSRC src/python/*cpp)
endif()

# Get executable files
if(BBM_BINARIES)
  file(GLOB EXECLIST bin/*cpp)
endif()

#########################################################################
# Scan for precomputation code
#########################################################################
message(STATUS "Setup Precomputation:")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
add_custom_target(precompute)

# scan for precomputations
file(GLOB PRECOMPDIR LIST_DIRECTORIES true ${BBM_SOURCE_DIR}/precompute/*)

# perform precomputations for each 
foreach(SUBDIR ${PRECOMPDIR})
  if(IS_DIRECTORY ${PRECOMPDIR})
       
   get_filename_component(PRECOMP_NAME ${SUBDIR} NAME)
   message(STATUS "Adding precomputation: ${SUBDIR}")
   list(APPEND CMAKE_MESSAGE_INDENT "  ")

   add_subdirectory(${SUBDIR})

   list(POP_BACK CMAKE_MESSAGE_INDENT)
  endif()
endforeach()

list(POP_BACK CMAKE_MESSAGE_INDENT)


#########################################################################
# Set Compile Targets 
#########################################################################
message(STATUS "Setting Up Compile Targets:")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

# python libraries 
if(BBM_PYTHON_LIBRARIES AND BBM_BACKBONE_CONFIGURATIONS AND NOT PYSRC STREQUAL "")

  # check list of target configurations; if not defined add all.
  if(NOT DEFINED BBM_PYTHON_CONFIG)
    message(STATUS "No BBM_PYTHON_CONFIG set; adding all configurations to compile targets")
    set(BBM_PYTHON_CONFIG ${BBM_BACKBONE_CONFIGURATIONS})
  endif()

  # for each target configuration
  foreach(NAME IN LISTS BBM_PYTHON_CONFIG)

     # check if config is valid
     if(${NAME} IN_LIST BBM_BACKBONE_CONFIGURATIONS)

       # add library
       set(PYLIBNAME ${BBM_NAME}_${NAME})
       pybind11_add_module(${PYLIBNAME} ${PYSRC})
       target_compile_definitions(${PYLIBNAME} PUBLIC BBM_CONFIG=${NAME} INTERFACE ${BBM_NAME})
       target_include_directories(${PYLIBNAME} PUBLIC ${BBM_NAME})
       target_link_libraries(${PYLIBNAME} PUBLIC ${BBM_NAME})
       message(STATUS "Adding python lib: ${PYLIBNAME}")  

       # make sure all precomputation have happened
       add_dependencies(${PYLIBNAME} precompute)

     # skip invalid configs
     else()
       message(STATUS "Ignore python lib: ${BBM_NAME}_${NAME} [unsupported configuration]")
     endif()
  endforeach()
else()
  message(STATUS "Skipping stand-alone Python libraries")
endif()

# executables
foreach(EXEC ${EXECLIST})
  get_filename_component(EXECNAME ${EXEC} NAME_WE)
  add_executable(${EXECNAME} ${EXEC})

  target_compile_definitions(${EXECNAME} INTERFACE ${BBM_NAME})
  target_include_directories(${EXECNAME} PUBLIC ${BBM_NAME})
  target_link_libraries(${EXECNAME} PUBLIC ${BBM_NAME})

  # make sure all precomputation have happened
  add_dependencies(${EXECNAME} precompute)

  message(STATUS "Adding executable: ${EXECNAME}")
endforeach(EXEC)

list(POP_BACK CMAKE_MESSAGE_INDENT)

#########################################################################
# Documentation
#########################################################################
if(PROJECT_IS_TOP_LEVEL)
  message(STATUS "Adding Documentation Generation: 'docs' target")
  add_subdirectory("docs" EXCLUDE_FROM_ALL)
else()
  message(STATUS "Skipping Document Generation (BBM used as subproject orlibrary)")
endif()


#########################################################################
# Process Exporters 
#########################################################################
if(BBM_EXPORTERS)
  message(STATUS "Processing Exporters:")
  list(APPEND CMAKE_MESSAGE_INDENT "  ")

  # get exporter names
  file(GLOB EXPDIR LIST_DIRECTORIES true ${BBM_SOURCE_DIR}/export/*)

  # add each subdirectory 
  foreach(SUBDIR ${EXPDIR})
    if(IS_DIRECTORY ${SUBDIR})
       
       get_filename_component(EXP_NAME ${SUBDIR} NAME)
       message(STATUS "Adding exporter: ${SUBDIR}")
       list(APPEND CMAKE_MESSAGE_INDENT "  ")

       add_subdirectory(${SUBDIR})

       list(POP_BACK CMAKE_MESSAGE_INDENT)
     endif()
  endforeach()

  list(POP_BACK CMAKE_MESSAGE_INDENT)
else()
  message(STATUS "Skipping exporters (BBM_EXPORTERS=OFF)")
endif()
