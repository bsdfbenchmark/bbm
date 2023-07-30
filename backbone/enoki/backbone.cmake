#########################################################################
# BBM Enoki Backbone
#########################################################################

#########################################################################
# Configuration Options
#########################################################################
if(NOT DEFINED ENOKI_AUTODIFF)
  message(STATUS "ENOKI_AUTODIFF not set; setting to default (ON)")
  set(ENOKI_AUTODIFF ON)
endif()

if(NOT DEFINED ENOKI_CUDA)
  message(STATUS "ENOKI_CUDA not set; setting to default (OFF)") 
  set(ENOKI_CUDA OFF)
endif()

if(NOT DEFINED ENOKI_PYTHON)
  message(STATUS "ENOKI_PYTHON not set; setting to default (OFF)")
  set(ENOKI_PYTHON OFF)
endif()

if(NOT ENOKI_PATH)
  message(STATUS "ENOKI_PATH not set; using default path.")
  set(ENOKI_PATH ${BBM_SOURCE_DIR}/ext/enoki)
endif()


#########################################################################
# Set available configurations
#########################################################################
set(BBM_BACKBONE_CONFIGURATIONS "floatRGB" "doubleRGB" "floatPacketRGB" "doublePacketRGB")

if(ENOKI_AUTODIFF)
  list(APPEND BBM_BACKBONE_CONFIGURATIONS "floatDiffRGB" "doubleDiffRGB")
endif()


#########################################################################
# Try to include enoki
#########################################################################
if(NOT EXISTS "${ENOKI_PATH}/CMakeLists.txt")
  message(FATAL_ERROR "Enoki library not found at ${ENOKI_PATH}; Set ENOKI_PATH in config.cmake")
  return()
endif()

# set PYBIND DIR
set(ENOKI_PYBIND11_DIR "${PYBIND11_INCLUDE_DIRS}"  CACHE STRING " " FORCE)

# set ENOKI OPTIONS
set(ENOKI_AUTODIFF "${ENOKI_AUTODIFF}" CACHE STRING " " FORCE)
set(ENOKI_CUDA "${ENOKI_CUDA}" CACHE STRING " " FORCE)
set(ENOKI_PYTHON "${ENOKI_PYTHON}" CACHE STRING " " FORCE)

# add enoki
add_subdirectory(${ENOKI_PATH} enoki)

# Update the compilation flags
enoki_set_compile_flags()
enoki_set_native_flags()

#########################################################################
# Import Dependendencies 
#########################################################################
target_include_directories(${BBM_NAME} SYSTEM INTERFACE ${ENOKI_PATH}/include ${BBM_SOURCE_DIR}/backbone/enoki/include)  ## use SYSTEM to avoid enoki warnings

if(ENOKI_AUTODIFF)
  target_compile_definitions(${BBM_NAME} INTERFACE ENOKI_AUTODIFF)
  target_link_libraries(${BBM_NAME} INTERFACE enoki-autodiff)
endif()

if(ENOKI_CUDA)
  target_compile_definitions(${BBM_NAME} INTERFACE ENOKI_CUDA)
  target_link_libraries(${BBM_NAME} INTERFACE enoki-cuda)  # to be tested!
endif()

if(ENOKI_PYTHON)
  target_compile_definitions(${BBM_NAME} INTERFACE ENOKI_PYTHON)
  # TODO link to enoki python lib
endif()



