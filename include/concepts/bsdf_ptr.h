#ifndef _BBM_BSDF_PTR_CONCEPT_H_
#define _BBM_BSDF_PTR_CONCEPT_H_

#include "concepts/bsdf.h"
#include "concepts/ptr_wrapper.h"

/************************************************************************/
/*! \file bsdf_ptr.h
  \brief bsdf_ptr contract
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief bsdf_ptr concpt

      A bsdf_ptr wraps a shared_ptr around a bsdf_base object. It provides the
      following minimum functionality:
      + concepts::bsdf
      + concepts::ptr_wrapper
    *********************************************************************/
    template<typename T>
      concept bsdf_ptr = concepts::bsdf<T> && concepts::ptr_wrapper<T, bsdf_base<typename std::decay_t<T>::Config>>;

  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_BSDF_PTR_CONCEPT_H_ */
