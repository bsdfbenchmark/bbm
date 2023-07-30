#if !defined _BBM_ENOKI_GRADIENT_H_ && defined ENOKI_AUTODIFF
#define _BBM_ENOKI_GRADIENT_H_

#include "backbone/type_traits.h"

#include "enoki/autodiff.h"


/************************************************************************/
/*! \file gradient.h
  
  \brief Enoki backbone support for auto-diff

  Only defined if ENOKI_AUTODIFF is set.
  
  Satisfies: concepts::backbone::gradient
*************************************************************************/

namespace backbone {

    //! \brief Enable/Disable tracking of gradients
    template<typename T> requires is_diff_v<T>
      void track_gradient(T& t, bool toggle=true)
    {
      enoki::set_requires_gradient(t, toggle);
    }

    //! \brief Returns true if the gradient is tracked.
    template<typename T> requires is_diff_v<T>
      bool is_gradient_tracked(const T& t)
    {
      return enoki::requires_gradient(t);
    }

    //! \brief Returns the gradient
    template<typename T> requires is_diff_v<T>
      auto gradient(T& t)
    {
      return enoki::gradient(t);
    }

    //! \brief Detach the value from gradient computations
    template<typename T> requires is_diff_v<T>
      auto detach_gradient(T& t)
    {
      return enoki::detach(t);
    }

    //! \brief Enable Forward Mode auto-diff
    template<typename T> requires is_diff_v<T>
      void forward_gradient(T& t)
    {
      enoki::forward(t);
    }

    //! \brief Enable Reverse Mode auto-diff
    template<typename T> requires is_diff_v<T>
      void backward_gradient(T& t)
    {
      enoki::backward(t);
    }
  
} // end backbone namespace

#endif /* _BBM_ENOKI_GRADIENT_H_ */
