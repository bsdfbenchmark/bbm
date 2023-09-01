#ifndef _BBM_LOSSFUNCTION_CONCEPT_H_
#define _BBM_LOSSFUNCTION_CONCEPT_H_

#include <utility>
#include "concepts/macro.h"

#include "bbm/config.h"

/************************************************************************/
/*! \file lossfunction.h

  \brief loss function contract

*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief loss function concept

      Each loss function requires:
      + concepts::has_config
      + update(void) method that initializes the loss. This method should be called at the beginning of each optimization step()
      + Value_t operator()(Mask=true) const method that returns the loss.
      + The returned loss should support addition.
    *********************************************************************/
    template<typename LOSSFUNC>
      concept lossfunction = requires(std::decay_t<LOSSFUNC>& func)
    {
      requires concepts::has_config<LOSSFUNC>;
      { func.update() };
      { std::as_const(func)() } -> std::same_as<Value_t<LOSSFUNC>>;
      { std::as_const(func)(std::declval<typename std::decay_t<Mask_t<LOSSFUNC>>>()) } -> std::same_as<Value_t<LOSSFUNC>>;
    };

    namespace archetype {
      /******************************************************************/
      /*! \brief lossfunction archetype for concept checking

        \tparam CONF = config to check for. Default = archetype::congif
      ******************************************************************/
      template<typename CONF=config> requires concepts::config<CONF>
        struct lossfunction
      {
        using Config = CONF;
        void update(void);
        Value_t<Config> operator()(Mask_t<Config> = true) const;
      };
    } // end archetype namespace

    BBM_CHECK_CONCEPT(bbm::concepts::lossfunction, archetype::lossfunction<>);
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_LOSSFUNCTION_CONCEPT_H_ */
