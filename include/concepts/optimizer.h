#ifndef _BBM_OPTIMIZER_CONCEPT_H_
#define _BBM_OPTIMIZER_CONCEPT_H_

#include <type_traits>
#include "concepts/config.h"
#include "concepts/optimization_algorithm.h"

/************************************************************************/
/*! \file optimizer.h
  \brief Optimizer contract: virtual interface wrapper around optimization_algorithms
************************************************************************/

namespace bbm {

  //! \brief Forward declaration
  template<typename CONF> requires concepts::config<CONF> struct optimizer_base;
  
  namespace concepts {

    /********************************************************************/
    /*! \brief optimizer concept

      Each optimizer must:
      + concepts::optimization_algorithm
      + inherit from optimizer_base
    *********************************************************************/
    template<typename T>
      concept optimizer = concepts::optimization_algorithm<T> && std::derived_from<std::decay_t<T>, bbm::optimizer_base<typename std::decay_t<T>::Config>>;
  
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_OPTIMIZER_CONCEPT_H_ */
