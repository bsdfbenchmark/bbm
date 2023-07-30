#ifndef _BBM_LOSS_PTR_CONCEPT_H_
#define _BBM_LOSS_PTR_CONCEPT_H_

#include "concepts/loss.h"
#include "concepts/ptr_wrapper.h"

/************************************************************************/
/*! \file loss_ptr.h
  \brief loss_ptr contract
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief loss_ptr concept

      A loss_ptr wraps a shared_ptr around a loss_base object. It provides
      the following guarantees:
      + concepts::loss
      + concepts::ptr_wrapper
    *********************************************************************/
    template<typename T>
      concept loss_ptr = concepts::loss<T> && concepts::ptr_wrapper<T, loss_base<typename std::decay_t<T>::Config>>;

  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_LOSS_PTR_CONCEPT_H_ */
