#ifndef _BBM_OPTIMIZATION_ALGORITHM_CONCEPT_H_
#define _BBM_OPTIMIZATION_ALGORITHM_CONCEPT_H_

#include "bbm/config.h"

/************************************************************************/
/*! \file optimization_algorithm.h
  \brief Optimization algorithm interface contract
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief optimization_algorithm concept

      Each optimization algorithm has the following:
      + concepts::config
      + Value step(void) : take one optimization step
      + void reset(void) : reset the internal state of the algorithm
      + Mask is_converged(void) const : true if the algorithm has converged
      
    *******************************************************************/
    template<typename OPT>
      concept optimization_algorithm = requires(OPT& opt)
    {
      requires concepts::config<OPT>;
      
      //! \brief step function
      { opt.step() } -> std::same_as<Value_t<OPT>>;

      //! \brief reset
      { opt.reset() };

      //! \brief is_converged
      { std::as_const(opt).is_converged() } -> std::same_as<Mask_t<OPT>>;
    };

    /********************************************************************/
    /*! \brief optimization_algorithm archetype for concept checking

      \tparam CONF = config to check for. Default = archetype::config
    *********************************************************************/
    namespace archetype {
      template<typename CONF=config>
        struct optimization_algorithm
      {
        using Config = CONF;
        Value_t<Config> step(void);
        void reset(void);
        Mask_t<Config> is_converged(void) const;
      };
    } // end archetype namespace

    BBM_CHECK_CONCEPT(concepts::optimization_algorithm, archetype::optimization_algorithm<>);
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_OPTIMIZER_CONCEPT_H_ */
