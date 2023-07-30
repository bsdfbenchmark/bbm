#ifndef _BBM_SAMPLEDLOSS_PTR_CONCEPT_H_
#define _BBM_SAMPLEDLOSS_PTR_CONCEPT_H_

#include "concepts/sampledloss.h"

/************************************************************************/
/*! \file sampledloss_ptr.h
  \brief sampledloss_ptr contract
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief sampledloss_ptr concept

      A sampledloss_ptr wraps a shared_ptr around a sampledloss_base object. It provides
      the following guarantees:
      + concepts::sampledloss
      + concepts::loss_ptr
    *********************************************************************/
    template<typename T>
      concept sampledloss_ptr = concepts::sampledloss<T> && concepts::loss_ptr<T>;

  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_SAMPLEDLOSS_PTR_CONCEPT_H_ */
