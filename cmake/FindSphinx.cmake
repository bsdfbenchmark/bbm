#Look for an executable called sphinx-build
find_program(SPHINX_EXECUTABLE
             NAMES sphinx-build
             HINTS $ENV{SPHINX_DIR} PATH_SUFFIXES bin
             DOC "Sphinx documentation generator")

include(FindPackageHandleStandardArgs)

#Handle standard arguments to find_package like REQUIRED and QUIET
find_package_handle_standard_args(Sphinx
                                  DEFAULT_MSG
                                  SPHINX_EXECUTABLE)

mark_as_advanced(SPHINX_EXECUTABLE)
