#ifndef _BBM_DRJIT_BACKBONE_H_
#define _BBM_DRJIT_BACKBONE_H_

#include <cstdint>
#include "util/string_literal.h"

#include "drjit/fwd.h"
#include "drjit/jit.h"
#include "drjit/math.h"
#include "drjit/array.h"
#include "drjit/packet.h"

#include "backbone/vec.h"
#include "backbone/color.h"
#include "backbone/complex.h"
#include "backbone/type_traits.h"
#include "backbone/math.h"
#include "backbone/horizontal.h"
#include "backbone/control.h"
#include "backbone/random.h"

#ifdef DRJIT_AUTODIFF
  #include "backbone/gradient.h"
#endif /* DRJIT_AUTODIFF */

/************************************************************************/
/*! \file backbone.h

  \brief Define the available configurations of the DrJIT backbone

*************************************************************************/

namespace bbm {
  
  /*** Implementation detail ***/
  namespace detail {
    template<typename VALUE, string_literal NAME>
      struct rgbConfig
    {
      static constexpr string_literal name = NAME;
      using Config = rgbConfig<VALUE,NAME>;
      using Value = VALUE;
      using Spectrum = backbone::color<Value>;
      static Spectrum wavelength(void) { return {0.645, 0.526, 0.444}; } // in micron
    };
  }

  /**********************************************************************/
  /*! @{ \name Config with non-packet/non-diff floating point RGB colors
   **********************************************************************/
  struct floatRGB : public detail::rgbConfig<DRJIT_FLOAT, "floatRGB"> {};
  struct doubleRGB : public detail::rgbConfig<DRJIT_DOUBLE, "doubleRGB"> {};
  //! @}

#ifdef DRJIT_AUTODIFF
  /**********************************************************************/
  /*! @{ \name Config with differentiable floating point RGB colors
   **********************************************************************/
  struct floatDiffRGB : public detail::rgbConfig<drjit::DiffArray<DRJIT_FLOAT>, "floatDiffArray"> {};
  struct doubleDiffRGB : public detail::rgbConfig<drjit::DiffArray<DRJIT_DOUBLE>, "doubleDiffArray"> {};
  //! @}
#endif /* DRJIT_AUTODIFF */
  
} // end bbm namespace


#ifdef DRJIT_BACKBONE_LLVM
  /**********************************************************************/
  /*! \brief Init the JIT if the DrJIT backbone is LLVM and AUTODIFF is enabled
  ***********************************************************************/
  struct DRJIT_LLVM_Backbone
  {
    DRJIT_LLVM_Backbone(void) { jit_init((uint32_t) JitBackend::LLVM ); }
  };

  static DRJIT_LLVM_Backbone _drjit_llvm_backbone;
#endif /* DRJIT_BACKBONE_LLVM */


#endif /* _BBM_DRJIT_BACKBONE_H_ */
