#ifndef _BBM_RANDOM_BACKBONE_CONCEPT_H_
#define _BBM_RANDOM_BACKBONE_CONCEPT_H_

#include <concepts>

/************************************************************************/
/*! \file random.h

  \brief Random number generator
*************************************************************************/

namespace bbm {
  namespace concepts {
    namespace backbone {

      /******************************************************************/
      /*! \brief Concept to check a randum number generator

        Each rng contains:
        + typedef type: basis type 
        + type operator(): generates a random number between lower and upper
        *****************************************************************/
      template<typename RNG>
        concept is_rng = requires(RNG& rng)
      {
        typename std::decay_t<RNG>::type;
        { rng() } -> std::same_as<typename std::decay_t<RNG>::type>;
      };
      
    } // end backbone namespace
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_RANDOM_BACKBONE_CONCEPT_H_ */

