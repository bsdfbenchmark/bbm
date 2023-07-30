# Compile the documentation with Sphinx+Doxygen+Breathe
#########################################################################

cmake_minimum_required(VERSION 3.21)
project(BBM_DOC)

########################################################################
# Setup files
########################################################################
file(GLOB_RECURSE BBM_INCLUDE_FILES ${PROJECT_SOURCE_DIR}/../include/*h)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake" ${CMAKE_MODULE_PATH})
set(DOXYGEN_SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")


#########################################################################
# Packages
#########################################################################
Find_Package(Doxygen QUIET)
Find_Package(Sphinx QUIET)

#########################################################################
# Setup Doxygen
#########################################################################
if(Doxygen_FOUND)

  # setup doxygen
  set(DOXYGEN_INPUT_DIR "${DOXYGEN_SOURCE_DIR}/include ${DOXYGEN_SOURCE_DIR}/backbone")
  set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/html/doxygen)
  set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/config.doxygen)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)

  # Print DEBUG INFO
  message(STATUS "${DOXYGEN_INPUT_DIR}")
  message(STATUS "${DOXYGEN_OUTPUT_DIR}")
  message(STATUS "${DOXYFILE_IN}")
  message(STATUS "${DOXYFILE_OUT}")
  message(STATUS "${Sphinx_FOUND}")
  message(STATUS "${READTHEDOCS}")
  

  # if no sphinx is found, setup doxygen as stand-alone
  if(READTHEDOCS OR Sphinx_FOUND)
    message(STATUS "Doxygen including Sphinx XML")
    set(DOXYGEN_GENERATE_TREE_VIEW "NO")
    set(DOXYGEN_GENERATE_XML "YES")
    set(DOXYGEN_DISABLE_INDEX "YES")    
    set(DOXYGEN_HEADER "${DOXYGEN_SOURCE_DIR}/docs/header.html")
    set(DOXYGEN_FOOTER "${DOXYGEN_SOURCE_DIR}/docs/footer.html")
  else()
    message(STATUS "Stand-alone Doxygen Documentation.")
    set(DOXYGEN_GENERATE_TREEVIEW "YES")
    set(DOXYGEN_GENERATE_XML "NO")
    set(DOXYGEN_DISABLE_INDEX "NO")
    set(DOXYGEN_HEADER "")
    set(DOXYGEN_FOOTER "") 
  endif()

  # Replace Variables inside @@ with current values
  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  # define a custom target
  file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
  add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                     DEPENDS ${BBM_INCLUDE_FILES}
                     COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                     COMMENT "Generating Docs")

  add_custom_target(doxygen DEPENDS ${DOXYGEN_INDEX_FILE})
else()
  message(STATUS "Doxygen not found. Skipping document generation")
endif()

#########################################################################
# Setup Sphinx
#########################################################################
if(Sphinx_FOUND AND Doxygen_FOUND)
  # setup sphinx
  set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
  set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/html)

  # add custom compile target
  add_custom_target(sphinx
                    COMMAND
                    ${SPHINX_EXECUTABLE} -b html -Dbreathe_projects.bbm=${DOXYGEN_OUTPUT_DIR}/xml
                    ${SPHINX_SOURCE} ${SPHINX_BUILD}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    COMMENT "Generating documentation with Sphinx")
else()
  message(STATUS "Sphinx not found. Skipping document generation")
endif()

#########################################################################
# Create 'docs' target
#########################################################################
if(Sphinx_FOUND OR Doxygen_FOUND)
   add_custom_target(docs)
   if(Doxygen_FOUND)
     add_dependencies(docs doxygen)
   endif()
   if(Sphinx_FOUND AND Doxygen_FOUND)
     add_dependencies(docs sphinx)
     add_dependencies(sphinx doxygen)
   endif()
endif()
