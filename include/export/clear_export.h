/***********************************************************************/
/*! \file clear_export.h
    \brief Clears and defines the export macros

    BBM_EXPORT_* should be included outside the compile header guards.
    The default state of the BBM_EXPORT_* macros is to do nothing.

    When overwriting the MACROs, including the afected header files will
    result in executing the macro.  Please see bbm_python.h for an example.
  
************************************************************************/

#undef BBM_EXPORT_BSDFMODEL
#define BBM_EXPORT_BSDFMODEL(bsdfmodel)

