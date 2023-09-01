#ifndef _BBM_BSDF_IMPORT_H_
#define _BBM_BSDF_IMPORT_H_

#include "concepts/config.h"
#include "bbm/bsdf_ptr.h"

/************************************************************************/
/*! \file bsdf_import.h

  \brief Import a BSDF from a string using the BBM_DEFAULT_BSDF_IMPORTER
  method specified in the configuration.
************************************************************************/

#ifdef BBM_STRING_BSDF_IMPORTER

#include "core/stringconvert.h"

namespace bbm {
  /*********************************************************************/
  /*! \brief Import a bsdf_ptr from a string using bbm::fromString
   *********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    auto bsdf_import(const std::string& str)
  {
    return bbm::fromString<bsdf_ptr<CONF>>(str);
  }
} // end bbm namepsace
#endif /* BBM_STRING_BSDF_IMPORTER */


#if defined(BBM_PYTHON_BSDF_IMPORTER) and !defined(BBM_CONFIG)

#include "python/bbm_python_interpreter.h"

namespace bbm {
  /**********************************************************************/
  /*! \brief Import a bsdf_ptr using the python interpreter
   **********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    auto bsdf_import(const std::string& str)
  {
    return python::capture<bsdf_ptr<CONF>>(str);
  }
} // end bbm namespace
#endif /* BBM_PYTHON_BSDF_IMPORTER && BBM_CONFIG */


#if defined(BBM_PYTHON_BSDF_IMPORTER) and defined(BBM_CONFIG)

#include "python/bbm_embed_module.h"

namespace bbm {
  /**********************************************************************/
  /*! \brief Import a bsdf_ptr using the EMBEDDED python interpreter

    Note: CONF must match the embdded configuration (BBM_CONFIG)
   **********************************************************************/
  template<typename CONF> requires concepts::config<CONF> && concepts::matching_config<CONF, BBM_CONFIG>
    auto bsdf_import(const std::string& str)
  {
    return embed::capture<bsdf_ptr<CONF>>(str);
  }
} // end bbm nanespace
#endif  /* BBM_PYTHON_BSDF_IMPORTER */
  
  
#endif /* _BBM_BSDF_IMPORT_H */
