#ifndef _BBM_LOSS_CONCEPT_H_
#define _BBM_LOSS_CONCEPT_H_

#include <type_traits>
#include "concepts/config.h"
#include "concepts/lossfunction.h"

/************************************************************************/
/*! \file loss.h
  \brief loss contract: virtual interface wrapper around lossfunctions 
*************************************************************************/

namespace bbm {

  //! \brief Forward declaration
  template<typename CONF> requires concepts::config<CONF> struct loss_base;

  namespace concepts {

    /********************************************************************/
    /*! \brief loss concept

      Each loss must:
      + concepts::config
      + concepts::lossfunction
      + inherit from loss_base
    *********************************************************************/
    template<typename T>
      concept loss = concepts::config<T> && concepts::lossfunction<T> && std::derived_from<std::decay_t<T>, bbm::loss_base<typename std::decay_t<T>::Config>>;

  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_LOSS_CONCEPT_H_ */
