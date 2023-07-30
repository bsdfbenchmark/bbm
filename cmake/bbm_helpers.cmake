#########################################################################
# BBM Cmake Helpers
#########################################################################

#########################################################################
# get_backbone_list: get the list of available backbones
#########################################################################
macro(get_backbone_list RESULT)
  set(${RESULT})
  file(GLOB BACKBONE_DIRS backbone/*)
  foreach(BDIR ${BACKBONE_DIRS})
    get_filename_component(NAME ${BDIR} NAME)
    list(APPEND ${RESULT} ${NAME})
  endforeach()
endmacro()


#########################################################################
# setup_backbone: set up the backbone
#########################################################################
macro(setup_backbone)
  if(NOT BBM_BACKBONE)
    set(BBM_BACKBONE "native")
    get_backbone_list(BACKBONE_LIST)
    message(STATUS "BBM_BACKBONE not set; using default 'native' backbone (available backbones are: ${BACKBONE_LIST}).")
  endif()
  
  if(NOT EXISTS ${BBM_SOURCE_DIR}/backbone/${BBM_BACKBONE})
    get_backbone_list(BACKBONE_LIST)
    message(FATAL_ERROR "Invalid backbone '${BBM_BACKBONE}'; set BBM_BACKBONE to one of: ${BACKBONE_LIST}.")
    return()
  endif()

  if(NOT EXISTS ${BBM_SOURCE_DIR}/backbone/${BBM_BACKBONE}/backbone.cmake )
    message(FATAL_ERROR "Backbone does not contain a valid 'backbone.cmake'")
    return()
  endif()

  include( ${BBM_SOURCE_DIR}/backbone/${BBM_BACKBONE}/backbone.cmake )

endmacro()


#########################################################################
# write_header: generate bbm_${SUB_DIR}.h containing includes to all header
#               files in the ${SUB_DIR}
#########################################################################
function(write_header BASE_DIR SUB_DIR)
  set(FNAME "${BASE_DIR}/bbm_${SUB_DIR}.h")
  message(STATUS "Generating '${FNAME}'")

  file(GLOB H_FILES ${BASE_DIR}/${SUB_DIR}/*h)

  file(WRITE  ${FNAME} "/***********************************************************************/\r\n")
  file(APPEND ${FNAME} "/*! \\file bbm_${SUB_DIR}.h\r\n")
  file(APPEND ${FNAME} "    \\brief Include all header files in '${SUB_DIR}'.\r\n\r\n")
  file(APPEND ${FNAME} "    Auto-generated file by cmake containing includes to all header\r\n")
  file(APPEND ${FNAME} "    files listed in '${SUB_DIR}'\r\n")
  file(APPEND ${FNAME} "************************************************************************/\r\n\r\n")
  
  foreach(H_FILE ${H_FILES})
     get_filename_component(NAME ${H_FILE} NAME)
     file(APPEND ${FNAME} "#include \"${SUB_DIR}/${NAME}\"\r\n")
  endforeach()

endfunction()


#########################################################################
# write_bbm_header: generate bbm.h that sets up BBM
#########################################################################
function(write_bbm_header BASE_DIR MODULE_DIRS)

  set(FNAME "${BASE_DIR}/bbm.h")
  message(STATUS "Generating '${FNAME}'")
  
  file(WRITE  ${FNAME} "/***********************************************************************/\r\n")
  file(APPEND ${FNAME} "/*! \\file bbm.h\r\n")
  file(APPEND ${FNAME} "    \\brief Include all modules and the backbone'.\r\n\r\n")
  file(APPEND ${FNAME} "    Auto-generated file by cmake.\r\n")
  file(APPEND ${FNAME} "************************************************************************/\r\n\r\n")
  file(APPEND ${FNAME} "#include \"bbm/bbm_core.h\"\r\n")
  
  foreach(MODULE ${MODULE_DIRS})
    file(APPEND ${FNAME} "#include \"bbm_${MODULE}.h\"\r\n")
  endforeach()

endfunction()
