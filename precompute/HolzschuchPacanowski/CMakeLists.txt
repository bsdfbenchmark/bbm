#########################################################################
# Precompute the tables for "A two-scale microfacet reflectance model
# combining reflection and diffraction", Holzschuch and Pacanowski 2017:
# https://doi.org/10.1145/3072959.3073621
#########################################################################

#########################################################################
# Initialization
#########################################################################
cmake_minimum_required(VERSION 3.15)

if(NOT BBM_SOURCE_DIR)
  message(FATAL_ERROR "Expected to be called from BBM's CMake script.")
  return()
endif()

#########################################################################
# Create Target dir
#########################################################################
set(PRECOMP_DIR ${BBM_SOURCE_DIR}/include/precomputed/holzschuchpacanowski)
file(MAKE_DIRECTORY ${PRECOMP_DIR})

#########################################################################
# Get list of precompute cpp files (assume each contains a precompute program)
#########################################################################
file(GLOB PREXECLIST *cpp)

#########################################################################
# Compile and call
#########################################################################
set(NAMELIST "")
foreach(PREXEC IN LISTS PREXECLIST)

  # base name
  get_filename_component(NAME ${PREXEC} NAME_WE)
  set(PRECOMP_FILE ${PRECOMP_DIR}/${NAME}.h)

  # if not exists; add to precomputation list
  if(NOT EXISTS ${PRECOMP_FILE})

    add_executable(${NAME} ${PREXEC})
    target_compile_definitions(${NAME} INTERFACE ${BBM_NAME})
    target_include_directories(${NAME} PUBLIC ${BBM_NAME})
    target_link_libraries(${NAME} PUBLIC ${BBM_NAME})

    # Run precomputations after build; after trigger rerun of cmake generation
    # to avoid precomputing again after a 'make clean'
    add_custom_command(TARGET ${NAME} POST_BUILD 
                       COMMAND ${NAME} ${PRECOMP_FILE}
                       COMMAND ${CMAKE_COMMAND} -E touch ${BBM_SOURCE_DIR}/CMakeLists.txt
                       VERBATIM
                       COMMENT "Precomputing: ${PRECOMP_FILE}"
                       #OUTPUT ${PRECOMP_FILE}
                       DEPENDS ${PREXEC}
    )

    list(APPEND NAMELIST ${NAME})

  endif()
endforeach()

#########################################################################
# Add to precompute target
#########################################################################
if(NAMELIST)
  add_dependencies(precompute ${NAMELIST})
endif()
