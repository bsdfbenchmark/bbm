#ifndef _BBM_OPTIMIZER_PTR_CONCEPT_H_
#define _BBM_OPTIMIZER_PTR_CONCEPT_H_

#include "concepts/optimizer.h"
#include "concepts/ptr_wrapper.h"

/************************************************************************/
/*! \file optimizer_ptr.h
  \brief optimizer_ptr contract
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief optimizer_ptr contract

      A optimizer_ptr wraps a shared_ptr around an optimizer_base object.
      It provides the following guarantees:
      + concepts::optimizer
      + concepts::ptr_wrapper
    *********************************************************************/
    template<typename T>
      concept optimizer_ptr = concepts::optimizer<T> && concepts::ptr_wrapper<T, optimizer_base<typename std::decay_t<T>::Config>>;
      
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_OPTIMIZER_PTR_CONCEPT_H_ */
