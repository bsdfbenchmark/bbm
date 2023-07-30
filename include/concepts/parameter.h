#ifndef _BBM_PARAMETER_CONCEPT_H_
#define _BBM_PARAMETER_CONCEPT_H_

#include "concepts/util.h"
#include "bbm/config.h"
#include "util/vector_util.h"

/************************************************************************/
/*! \file parameter.h
  \brief Concepts related to BSDF model parameters
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief Basic parameter concept

      A parameter list must be:
      + std::ranges::range
      + concepts::has_basic_math
    ********************************************************************/
    template<typename PARAM>
      concept parameter = std::ranges::range<PARAM> && concepts::has_basic_math<PARAM>;

    
    namespace archetype {
      //! \brief Archetype of a non-differentiable parameter
      template<typename CONF=concepts::archetype::config>
        using parameter = bbm::vector<Value_t<CONF>>;
    } // end archetype namespace

    
    BBM_CHECK_CONCEPT( concepts::parameter, archetype::parameter<> );

    
    /********************************************************************/
    /*! \brief Differentiable parameter concept

      A differentiable parameter list must be:
      + concepts::parameter
      + its value type must support differentiation
    *********************************************************************/
    template<typename PARAM>
      concept diff_parameter = requires(PARAM param)
    {
      requires concepts::parameter<PARAM>;
      requires is_diff_v< decltype(*std::begin(param)) >;
    };


    namespace archetype {
      //! \brief Archetype of a differentiable parameter
      template<typename CONF=concepts::archetype::config>
        using diff_parameter = bbm::vector<add_diff_t<Value_t<CONF>>>;
    } // end archetype namespace
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_PARAMETER_CONCEPT_H_ */
