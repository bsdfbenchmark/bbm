#ifndef _BBM_COMPLEX_BACKBONE_CONCEPT_H_
#define _BBM_COMPLEX_BACKBONE_CONCEPT_H_

#include <concepts>

/************************************************************************/
/*! \file complex.h

  \brief Additional requirements for the complex data-type (in addition to all
  other backbone type requirements; see BBM_VALIDATE_BACKBONE in backbone.h).

*************************************************************************/

namespace bbm {
  namespace concepts {
    namespace backbone {

      /******************************************************************/
      /*! \brief Addiional requirements for complex data-type.
        *****************************************************************/
      template<typename T>
        concept has_complex_functions = requires(T a)
      {
        requires concepts::valid_trait<value_t, T>;
        
        { real(a) } -> std::convertible_to<value_t<T>>;
        { imag(a) } -> std::convertible_to<value_t<T>>;
        { conj(a) } -> std::same_as<T>;
      };

    } // end backbone namespace
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_COMPLEX_BACKBONE_CONCEPT_H_ */
