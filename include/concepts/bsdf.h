#ifndef _BBM_BSDF_CONCEPT_H_
#define _BBM_BSDF_CONCEPT_H_

#include <type_traits>
#include "concepts/config.h"
#include "concepts/bsdfmodel.h"

/************************************************************************/
/*! \file bsdf.h
  \brief bsdf contract: virtual interface wrapper around bsdfmodels
*************************************************************************/

namespace bbm {

  //! \brief Forward declaration
  template<typename CONF> requires concepts::config<CONF> struct bsdf_base;
    
  namespace concepts {
    
    /********************************************************************/
    /*! \brief bsdf concept

      Each bsdf must:
      + concepts::config
      + concepts::bsdfmodel
      + inherit from bsdf_base
    *********************************************************************/
    template<typename T>
      concept bsdf = concepts::config<T> && concepts::bsdfmodel<T> &&  std::derived_from<std::decay_t<T>, bbm::bsdf_base<typename std::decay_t<T>::Config>>;
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_BSDF_CONCEPT_H_ */
