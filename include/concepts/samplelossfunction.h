#ifndef _BBM_SAMPLELOSSFUNCTION_CONCEPT_H_
#define _BBM_SAMPLELOSSFUNCTION_CONCEPT_H_

#include "concepts/util.h"

#include "bbm/config.h"

/*************************************************************************/
/*! \file samplelossfunction_concept.h

  \brief sample loss function contract.  Sample loss functions are used in sampled loss functions

*************************************************************************/

namespace bbm {
  namespace concepts {
  
    /********************************************************************/
    /* ! \brief samplelossfunction concept
       
       A sample loss function computes the loss over a single sample:
       + concepts::config
       + Value operator()(const Vec3d& in, const Vec3d& out, const Spectrum& value, const Spectrum& reference) const
       + the result of the operator() must support addition

    *********************************************************************/
    template<typename SAMPLELOSS>
      concept samplelossfunction = requires(const SAMPLELOSS& loss)
    {
      requires concepts::config<SAMPLELOSS>;
      
      { loss(std::declval<Vec3d_t<SAMPLELOSS>>(), std::declval<Vec3d_t<SAMPLELOSS>>(), std::declval<Spectrum_t<SAMPLELOSS>>(), std::declval<Spectrum_t<SAMPLELOSS>>()) } -> std::same_as<Value_t<SAMPLELOSS>>;
      requires concepts::has_addition<Value_t<SAMPLELOSS>>;
    };

    /********************************************************************/
    /*! \brief samplelossfunction archetype for concept checking

      \tparam CONF = config to check for. Default = archetype::config
    *********************************************************************/
    namespace archetype {
      template<typename CONF=config> requires concepts::config<CONF>
        struct samplelossfunction
      {
        using Config = CONF;
        Value_t<Config> operator()(const Vec3d_t<Config>&, const Vec3d_t<Config>&, const Spectrum_t<Config>&, const Spectrum_t<Config>&) const;
      };
    } // end archetype namespace

    BBM_CHECK_CONCEPT(concepts::samplelossfunction, archetype::samplelossfunction<>);
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_SAMPLELOSSFUNCTION_CONCEPT_H_ */
