set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT TARGET BBM)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.." bbm)
    add_library(bbm::core ALIAS ${BBM_NAME})
endif()
