#ifndef _BBM_SAMPLEDLOSS_CONCEPT_H_
#define _BBM_SAMPLEDLOSS_CONCEPT_H_

#include <type_traits>

#include "concepts/loss.h"
#include "concepts/sampledlossfunction.h"

/************************************************************************/
/*! \file sampledloss.h

  \brief sampledloss contract

*************************************************************************/

namespace bbm {

  //! \brief Forward declaration
  template<typename CONF> requires concepts::config<CONF> struct sampledloss_base;

  namespace concepts {

    /********************************************************************/
    /*! \brief sampledloss concept

      Every sampled loss:
      + concepts::loss
      + concepts::sampledlossfunction
      + inherit from sampledloss_base
    *********************************************************************/
    template<typename T>
    concept sampledloss = concepts::loss<T> && concepts::sampledlossfunction<T> && std::derived_from<std::decay_t<T>, bbm::sampledloss_base<typename std::decay_t<T>::Config>>;

  } // end concepts namespace
} // end bbm namespace


#endif /* _BBM_SAMPLEDLOSS_CONCEPT_H_ */
