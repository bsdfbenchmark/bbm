#ifndef _BBM_SAMPLEDLOSSFUNCTION_CONCEPT_H_
#define _BBM_SAMPLEDLOSSFUNCTION_CONCEPT_H_

#include <utility>
#include "concepts/lossfunction.h"

/************************************************************************/
/*! \file sampledlossfunction.h

  \brief sampled loss function contract

*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief sampled loss function concept

      Each loss sampled function requires:
      + concepts::lossfunction
      + Size_t samples(void) const returns the number of samples.
      + Value_t operator(Size_t index, Mask=true) const that returns the loss of the index-the sample
      + The returned loss should support addition.
    *********************************************************************/
    template<typename LOSSFUNC>
      concept sampledlossfunction = requires(LOSSFUNC func)
    {
      requires concepts::lossfunction<LOSSFUNC>;
      
      { std::as_const(func).samples() } -> std::same_as<Size_t<LOSSFUNC>>;

      { std::as_const(func)(std::declval<Size_t<LOSSFUNC>>()) } -> std::same_as<Value_t<LOSSFUNC>>;
      { std::as_const(func)(std::declval<Size_t<LOSSFUNC>>(), std::declval<Mask_t<LOSSFUNC>>()) } -> std::same_as<Value_t<LOSSFUNC>>;
      requires concepts::has_addition<Value_t<LOSSFUNC>>;
    };

    /********************************************************************/
    /*! \brief sampledlossfunction archetype for concept checking

      \tparam CONF = config to check for. Default = archetype::config
    *********************************************************************/
    namespace archetype {
      template<typename CONF=config> requires concepts::config<CONF>
        struct sampledlossfunction : lossfunction<CONF>
      {
        using Config=CONF;
        using lossfunction<CONF>::operator();
        Size_t<Config> samples(void) const;
        Value_t<Config> operator()(const Size_t<Config>&, Mask_t<Config>) const;
      };
      
    } // end archetype namespace

    BBM_CHECK_CONCEPT(concepts::sampledlossfunction, archetype::sampledlossfunction<>);
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_LOSSFUNCTION_CONCEPT_H_ */
