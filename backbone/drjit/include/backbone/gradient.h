#if !defined _BBM_DRJIT_GRADIENT_H_ && defined DRJIT_AUTODIFF
#define _BBM_DRJIT_GRADIENT_H_

#include "backbone/type_traits.h"

#include "drjit/autodiff.h"


/************************************************************************/
/*! \file gradient.h
  
  \brief DrJIT backbone support for auto-diff

  Only defined if DRJIT_AUTODIFF is set.
  
  Satisfies: concepts::backbone::gradient
*************************************************************************/

namespace backbone {

    //! \brief Enable/Disable tracking of gradients
    template<typename T> requires is_diff_v<T>
      void track_gradient(T& t, bool toggle=true)
    {
      drjit::set_grad_enabled(t, toggle);
    }

    //! \brief Returns true if the gradient is tracked.
    template<typename T> requires is_diff_v<T>
      bool is_gradient_tracked(const T& t)
    {
      return drjit::grad_enabled(t);
    }

    //! \brief Returns the gradient
    template<typename T> requires is_diff_v<T>
      auto gradient(T& t)
    {
      return drjit::grad(t);
    }

    //! \brief Detach the value from gradient computations
    template<typename T> requires is_diff_v<T>
      auto detach_gradient(T& t)
    {
      return drjit::detach(t);
    }

    //! \brief Enable Forward Mode auto-diff
    template<typename T> requires is_diff_v<T>
      void forward_gradient(T& t)
    {
      drjit::forward(t);
    }

    //! \brief Enable Reverse Mode auto-diff
    template<typename T> requires is_diff_v<T>
      void backward_gradient(T& t)
    {
      drjit::backward(t);
    }
  
} // end backbone namespace

#endif /* _BBM_DRJIT_GRADIENT_H_ */
