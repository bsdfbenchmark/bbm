#########################################################################
# BBM DrJIT Backbone
#########################################################################

## TODO: set JIT targets to match requested DRJIT_BACKBONE
## TODO: python interface

#########################################################################
# Configuration Options
#########################################################################
if(NOT DEFINED DRJIT_AUTODIFF)
  message(STATUS "DRJIT_AUTODIFF not set; setting to default (ON)")
  set(DRJIT_AUTODIFF ON)
endif()

if(NOT DEFINED DRJIT_JIT)
  message(STATUS "DRJIT_JIT not set; setting to default (ON)") 
  set(DRJIT_JIT ON)
endif()

if(NOT DEFINED DRJIT_PYTHON)
  message(STATUS "DRJIT_PYTHON not set; setting to default (OFF)")
  set(DRJIT_PYTHON OFF)
endif()

if(NOT DEFINED DRJIT_PYTHON_PACKET)
  message(STATUS "DRJIT_PYTHON_PACKET not set; setting to default (OFF)")
  set(DRJIT_PYTHON_PACKET OFF)
endif()

if(NOT DRJIT_PATH)
  message(STATUS "DRJIT_PATH not set; using default path.")
  set(DRJIT_PATH ${BBM_SOURCE_DIR}/ext/drjit)
endif()

if(NOT DRJIT_BACKBONE)
  message(STATUS "DRJIT_BACKBONE not set; using default (scalar)")
  set(DRJIT_BACKBONE "scalar")
endif()

#########################################################################
# Set DrJIT backbone
#########################################################################
if(DRJIT_BACKBONE STREQUAL "scalar")
  target_compile_definitions(${BBM_NAME} INTERFACE DRJIT_BACKBONE_SCALAR DRJIT_FLOAT=float DRJIT_DOUBLE=double)
elseif(DRJIT_BACKBONE STREQUAL "LLVM")
  target_compile_definitions(${BBM_NAME} INTERFACE DRJIT_BACKBONE_LLVM DRJIT_FLOAT=drjit::LLVMArray<float> DRJIT_DOUBLE=drjit::LLVMArray<double>)
else()
  message(FATAL_ERROR "DrJIT backbone (${DRJIT_BACKONE}) not (yet) supported.")
endif()

#########################################################################
# Set available configurations
#########################################################################
set(BBM_BACKBONE_CONFIGURATIONS "floatRGB" "doubleRGB") ## "floatPacketRGB" "doublePacketRGB")

if(DRJIT_AUTODIFF)
  list(APPEND BBM_BACKBONE_CONFIGURATIONS "floatDiffRGB" "doubleDiffRGB")
endif()


#########################################################################
# Try to include drjit
#########################################################################
if(NOT EXISTS "${DRJIT_PATH}/CMakeLists.txt")
  message(FATAL_ERROR "DrJIT library not found at ${DRJIT_PATH}; Set DRJIT_PATH in config.cmake")
  return()
endif()

# set PYBIND DIR
#set(DRJIT_PYBIND11_DIR "${PYBIND11_INCLUDE_DIRS}"  CACHE STRING " " FORCE)

# set DRJIT OPTIONS
set(DRJIT_EANBLE_AUTODIFF "${DRJIT_AUTODIFF}" CACHE STRING " " FORCE)
set(DRJIT_ENABLE_JIT "${DRJIT_JIT}" CACHE STRING " " FORCE)
set(DRJIT_ENABLE_PYTHON "${DRJIT_PYTHON}" CACHE STRING " " FORCE)
set(DRJIT_ENABLE_PYTHON_PACKET "${DRJIT_PYTHON_PACKET}" CACHE STRING " " FORCE)

# Update the compilation flags
include(${DRJIT_PATH}/ext/drjit-core/ext/nanothread/ext/cmake-defaults/CMakeLists.txt)

#########################################################################
# set-up build DrJIT
#########################################################################
add_subdirectory(${DRJIT_PATH})

#########################################################################
# Import Dependendencies
#########################################################################
target_include_directories(${BBM_NAME} INTERFACE
                    ${DRJIT_PATH}/include
                    ${DRJIT_PATH}/ext/drjit-core/include
                    ${DRJIT_PATH}/ext/drjit-core/ext/nanothread/include
                    ${BBM_SOURCE_DIR}/backbone/drjit/include
)

if(DRJIT_JIT)
  target_compile_definitions(${BBM_NAME} INTERFACE DRJIT_JIT)
  target_link_libraries(${BBM_NAME} INTERFACE drjit-core)
endif()

if(DRJIT_AUTODIFF)
  target_compile_definitions(${BBM_NAME} INTERFACE DRJIT_AUTODIFF)
  target_link_libraries(${BBM_NAME} INTERFACE drjit-autodiff)
endif()

if(DRJIT_PYTHON)
  target_compile_definitions(${BBM_NAME} INTERFACE DRJIT_PYTHON)
  #target_link_libraries(${BBM_NAME} INTERFACE drjit-python)
  # TODO link to drjit python lib
endif()
