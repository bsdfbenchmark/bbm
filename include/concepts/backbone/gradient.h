#ifndef _BBM_GRADIENT_BACKBONE_CONCEPT_H_
#define _BBM_GRADIENT_BACKBONE_CONCEPT_H_

#include "concepts/util.h"

/************************************************************************/
/*! \file gradient.h

  \brief A differentiable type T has valid gradient operations if:
  + is_diff_v<T> is defined and is true
  + gradient(T& ) return a reference to the gradient
  + detach_gradient(T& ) returns a reference to the raw value (without gradient tracking)
  + track_gradient(T&, bool toggle) enable/disbale gradient tracking
  + is_gradient_tracked(const T&) true if gradients are tracked
  + forward_gradient(T&) set forward tracking mode
  + backward_gradient(T&) set backward tracking mode

*************************************************************************/

namespace bbm {
  namespace concepts {
    namespace backbone {

      template<typename T>
        concept gradient = requires(T a)
      {
        { is_diff_v<T> } -> std::convertible_to<bool>;
        requires is_diff_v<T>;

        /****************************************************************/
        /*! \brief track_gradient(T&, bool)

          Enable/disable tracking of gradients. 
        *****************************************************************/
        { track_gradient(a, true) };

        /****************************************************************/
        /*! \brief is_gradient_tracked(T&)

          Returns true if gradient tracking is enabled
        *****************************************************************/
        { is_gradient_tracked(a) } -> std::convertible_to<bool>;

        /****************************************************************/
        /*! \brief gradient(T&)

          Computes/returns the gradient of a variable (of computations since
          gradient tracking was enabled.
        *****************************************************************/
        { gradient(a) } -> std::convertible_to<remove_diff_t<T>>;

        /****************************************************************/
        /*! \brief detach_gradient(T&)

          Returns the raw value of the variable that can be altered without
          affecting the gradient.
        *****************************************************************/
        { detach_gradient(a) } -> std::convertible_to<remove_diff_t<T>>;

        /****************************************************************/
        /*! \brief forward_gradient(T&)

          Enable forward computation of the gradients of T.
        *****************************************************************/
        { forward_gradient(a) };

        /****************************************************************/
        /*! \brief backward_gradient(T&)

          Enable backward/revserse computation of the gradients of T.
        *****************************************************************/
        { backward_gradient(a) };
      };


      /******************************************************************/
      /*! \brief Only check gradient concept if is_diff_v<T>
       ******************************************************************/
      template<typename T>
        concept if_diff_gradient = (!is_diff_v<T>) || gradient<T>;
      
    } // end backbone namespace
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_GRADIENT_BACKBONE_CONCEPT_H_ */
