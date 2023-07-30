if(WIN32 OR APPLE)
  message(WARNING "Currently only tested on Linux; Likely needs tuning for a non-linux OS")
endif()


#########################################################################
# getMitsubaPath: find Mitsuba; uses MITSUBA_PATH and MITSUBA_BINRY_PATH as
#                 hints.
#########################################################################
macro(getMitsubaPath)

    # try to locate mitsuba
    if(EXISTS "${MITSUBA_PATH}/include/mitsuba/mitsuba.h")

        # Get mitsuba version (core/version.h only exists in Mitsuba 1)
        if(EXISTS "${MITSUBA_PATH}/include/mitsuba/core/version.h")
            file(READ "${MITSUBA_PATH}/include/mitsuba/core/version.h" VERSION_TXT)
            string(REGEX MATCH "MTS_VERSION \"([0-9]*\\.[0-9]*\\.[0-9]*)\"" _ ${VERSION_TXT})
            set(MTS_VERSION ${CMAKE_MATCH_1})
        else()
            message(STATUS "Mitsuba wrong version (${MITSUBA_PATH})")
	    return()
        endif()

        # success
        message(STATUS "Mitsuba ${MTS_VERSION} found.")

        # check binaries directory (if not set, gather all subdirs)
        if(NOT MITSUBA_BINARY_PATH)
            file(GLOB MITSUBA_BINARY_PATH LIST_DIRECTORIES true ${MITSUBA_PATH}/*)
        endif()

        find_path(MITSUBA_BINARY "libmitsuba-core.so" HINT ${MITSUBA_BINARY_PATH})

        if(MITSUBA_BINARY)
            set(MITSUBA_BINARY_PATH ${MITSUBA_BINARY})
            message(STATUS "Mitsuba binaries found: ${MITSUBA_BINARY_PATH}")
        else()   
            message(STATUS "Unable to locate precompiled Mitsuba binaries directory in (set MITSUBA_BINARY_PATH)")
            return()        
        endif()
    elseif(NOT MITSUBA_PATH)
        message(STATUS "Mitsuba: Not found (set MITSUBA_PATH); Skipping exporter")
        return()
    else()
        message(STATUS "Mitsuba not found at MITSUBA_PATH=${MITSUBA_PATH}; skipping exporter")
        return()
    endif()
endmacro()


#########################################################################
# getMitsubaCMakeVars: Grab relevant MTS variables from existing CMakeCache
#########################################################################
macro(getMitsubaCMakeVars filename)
    # read cache
    load_cache("${MITSUBA_PATH}" READ_WITH_PREFIX "BBM_" MTS_DEBUG MTS_KD_DEBUG MTS_KD_CONSERVE_MEMORY MTS_SINGLE_PRECISION MTS_DOUBLE_PRECISION MTS_SSE MTS_HAS_COHERENT_RT MTS_DEBUG_FRP MTS_SPECTRUM_SAMPLES CMAKE_CXXFLAGS)

    # copy
    set(MTS_DEBUG ${BBM_MTS_DEBUG})
    set(MTS_KD_DEBUG ${BBM_MTS_KD_DEBUG})
    set(MTS_KD_CONSERVE_MEMORY ${BBM_MTS_KD_CONSERVE_MEMORY})
    set(MTS_SINGLE_PRECISION ${BBM_MTS_SINGLE_PRECISION})
    set(MTS_DOUBLE_PRECISION ${BBM_MTS_DOUBLE_PRECISION})
    set(MTS_SSE ${BBM_MTS_SSE})
    set(MTS_HAS_COHERENT_RT ${BBM_MTS_HAS_COHERENT_RT})
    set(MTS_DEBUG_FP ${BBM_MTS_DEBUG_FP})
    set(MTS_SPECTRUM_SAMPLES ${BBM_MTS_SPECTRUM_SAMPLES})

    # check for unsupported flags
    if(BBM_MTS_USE_PCH OR BBM_MTS_USE_PCH_ALL_PLUGINS OR BBM_CMAKE_CONFIGURATION_TYPES)
      message(WARNING "The following options are ignored: MTS_USE_PCH, MTS_USE_PCH_ALL_PLUGINS, CMAKE_CONFIGURATION_TYPES")
    endif()

endmacro()

#########################################################################
# getMistubaDefaultVars: default Mitsuba settings
#########################################################################
macro(getMitsubaDefaultVars)
    # Default options
    option(MTS_DEBUG ON)
    option(MTS_KD_DEBUG OFF)
    option(MTS_KD_CONSERVE_MEMORY OFF)
    option(MTS_SINGLE_PRECISION ON)
    option(MTS_DOUBLE_PRECISION OFF)
    option(MTS_SSE ON)
    option(MTS_HAS_COHERENT_RT ON)
    option(MTS_DEBUG_FP OFF)

    if(NOT MTS_SPECTRUM_SAMPLES)
       set(MTS_SPECTRUM_SAMPLES 3)
    endif()

    # Platform-specific definitions
    if (MSVC OR (WIN32 AND CMAKE_C_COMPILER_ID MATCHES "Intel"))
       set(MTS_NO_CHECKED_ITERATORS  OFF)
       set(MTS_NO_ITERATOR_DEBUGGING OFF)
       option(MTS_NO_BUFFER_CHECKS ON)
    endif()

endmacro()

#########################################################################
# convertSconsFlags: parse a line from the SConds config script and add to target
#########################################################################
macro(convertSconsFlags target source)
                        
  # get all options
  if( ${source} MATCHES "MTS_DEBUG" )
      set(MTS_DEBUG ON)
  endif()

  if( ${source} MATCHES "MTS_KD_DEBUG" )
      set(MTS_KD_DEBUG ON)
  endif()

  if( ${source} MATCHES "MTS_KD_CONVSERVE_MEMORY" )
      set(MTS_KD_CONSERVE_MEMORY ON)
  endif()

  if( ${source} MATCHES "SINGLE_PRECISION" )
      set(MTS_SINGLE_PRECISION ON)
  endif()

  if( ${source} MATCHES "DOUBLE_PRECISION" )
      set(MTS_DOUBLE_PRECISION ON)
  endif()

  if( ${source} MATCHES "MTS_SSE" )
      set(MTS_SSE ON)
  endif()

  if( ${source} MATCHES "MTS_HAS_COHERENT_RT" )
      set(MTS_HAS_COHERENT_RT ON)
  endif()

  if( ${source} MATCHES "MTS_DEBUG_FP" )
      set(MTS_DEBUG_FP ON)
  endif()

  if( ${source} MATCHES "SPECTRUM_SAMPLES")
      string( REGEX MATCH "SPECTRUM_SAMPLES=([0-9]*)" SAMPSTR ${source})
      string( REGEX MATCH "([0-9]+)" SAMP ${SAMPSTR})
      set(MTS_SPECTRUM_SAMPLES ${SAMP})
  else()
      set(MTS_SPECTRUM_SAMPLES 3)
  endif()

  # check unsupported flags
  if(${source} MATCHES "MTS_USE_PCH" OR ${source} MATCHES "MTS_USE_PCH_ALL_PLUGINS")
    message(WARNING "The following options are ignored: MTS_USE_PCH, MTS_USE_PCH_ALL_PLUGINS")
  endif()
    
endmacro()

#########################################################################
# getMitsubaSConsVars: extract the Mitsuba com[ile settings from config.py
#########################################################################
macro(getMitsubaSConsVars filename)

  file(READ ${filename} SCONFIG)

  # reconstruct CXXFLAGS (including all options)
  string(REGEX MATCH "CXXFLAGS ([ \t\r\n]*) = \\[([^\]])*\\]" CXXSTRING ${SCONFIG})
  string(REGEX MATCH "CCFLAGS ([ \t\r\n]*) = \\[([.^\]]*)\\]" CCSTRING ${SCONFIG})    

  if(CXXSTRING)
    convertSconsFlags(MTS_CXX_FLAGS ${CXXSTRING})
  endif()

  if(CCSTRING)
    convertSconsFlags(MTS_CXX_FLAGS ${CCSTRING})
  endif()

endmacro()

#########################################################################
# printMitsubaVars: debug helper
#########################################################################
function(printMitsubaVars)
    message(STATUS "MTS_CXX_FLAGS=${MTS_CXX_FLAGS}")
    message(STATUS "MTS_DEBUG=${MTS_DEBUG}")
    message(STATUS "MTS_KD_DEBUG=${MTS_KD_DEBUG}")
    message(STATUS "MTS_KD_CONSERVE_MEMORY=${MTS_KD_CONSERVE_MEMORY}")
    message(STATUS "MTS_SINGLE_PRECISION=${MTS_SINGLE_PRECISION}")
    message(STATUS "MTS_DOUBLE_PRECISION=${MTS_DOUBLE_PRECISION}")
    message(STATUS "MTS_SSE=${MTS_SSE}")
    message(STATUS "MTS_HAS_COHERENT_RT=${MTS_HAS_COHERENT_RT}")
    message(STATUS "MTS_DEBUG_FP=${MTS_DEBUG_FP}")
    message(STATUS "MTS_SPECTRUM_SAMPLES=${MTS_SPECTRUM_SAMPLES}")
endfunction()


#########################################################################
# getMistibaCompileSettings: reverse engineer the compile setting of Mistuba
#########################################################################
macro(getMitsubaCompileSettings)
       
   # check if this was a CMake build
   if(EXISTS "${MITSUBA_BINARY_PATH}/CMakeCache.txt")
       message(STATUS "Found out-source CMakeCache; extracting compile settings")
       getMitsubaCMakeVars("${MITSUBA_BINARY_PATH}/CMakeCache.txt")

   elseif(EXISTS "${MITSUBA_PATH}/CMakeCache.txt")
       message(STATUS "Found in-source CMakeCache; extracting compile settings")
       getMitsubaCMakeVars("${MITSUBA_PATH}/CMakeCache.txt")

   # otherwise check if SCons was used
   elseif(EXISTS "${MITSUBA_PATH}/config.py")
       message(STATUS "Found 'config.py'; extracting compile settings")
       getMitsubaSConsVars("${MITSUBA_PATH}/config.py")

   # otherwise; give up and use default (or overrides by used via -D<mitsuba setting>
   else()
       message(WARNING "Unable to determine Mitsuba compile options. Continuing with default or manually set options.")
       getMistubaDefaultVars()
    endif()

    # debug
    #printMitsubaVars()

    # Set Compiler flags to suppress warnings; compiler specific
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")
    endif()

    # Add user set compiler options
    if(MITSUBA_COMPILER_OPTIONS)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MITSUBA_COMPILER_OPTIONS}")
    endif()

    # Top level configuration definitions
    if (MTS_DEBUG)
        add_compile_definitions(MTS_DEBUG)
    endif()

    if (MTS_KD_DEBUG)
        add_compile_definitions(MTS_KD_DEBUG)
    endif()

    if (MTS_KD_CONSERVE_MEMORY)
        add_compile_definitions(MTS_KD_CONSERVE_MEMORY)
    endif()

    if (MTS_SINGLE_PRECISION)
        add_compile_definitions(SINGLE_PRECISION)
    endif()

    if (MTS_DOUBLE_PRECISION)
        add_compile_definitions(DOUBLE_PRECISION)
    endif()

    if(MTS_SPECTRUM_SAMPLES)
        if(NOT "${MTS_SPECTRUM_SAMPLES}" MATCHES "^[1-9][0-9]*$" OR MTS_SPECTRUM_SAMPLES LESS 3 OR MTS_SPECTRUM_SAMPLES GREATER 2048)
            message(FATAL_ERROR "Invalid number of spectrum samples: ${MTS_SPECTRUM_SAMPLES}. Valid values: [3,2048]")
        else()
            add_compile_definitions(SPECTRUM_SAMPLES=${MTS_SPECTRUM_SAMPLES})
        endif()
    endif()

    if (MTS_SSE)
        add_compile_definitions(MTS_SSE)
    endif ()

    if (MTS_HAS_COHERENT_RT)
        add_compile_definitions(MTS_HAS_COHERENT_RT)
    endif()

    if (MTS_DEBUG_FP)
        add_compile_definitions(MTS_DEBUG_FP)
    endif()

endmacro()



#########################################################################
# bsdf_plugin: create an mitsuba bsdf plugin
#########################################################################
macro (bsdf_plugin _plugin_name _plugin_src)

  # create library
  add_library (${_plugin_name} MODULE ${_plugin_src})

  # set link libraries
  target_link_libraries (${_plugin_name} "mitsuba-core" "mitsuba-render" "mitsuba-hw" pybind11::embed)

  # windows & apple stuff
  set_target_properties (${_plugin_name} PROPERTIES PREFIX "")
  if (APPLE)
    set_target_properties (${_plugin_name} PROPERTIES SUFFIX ".dylib")
  endif ()
  if (WIN32)
    set_target_properties (${_plugin_name} PROPERTIES VERSION "${MTS_VERSION}")
  endif()

  # set target properties
  set_target_properties (${_plugin_name} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE 1)
endmacro ()
