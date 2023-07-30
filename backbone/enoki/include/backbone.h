#ifndef _BBM_ENOKI_BACKBONE_H_
#define _BBM_ENOKI_BACKBONE_H_

#include "util/string_literal.h"

#include "backbone/array.h"
#include "backbone/vec.h"
#include "backbone/color.h"
#include "backbone/complex.h"
#include "backbone/type_traits.h"
#include "backbone/math.h"
#include "backbone/horizontal.h"
#include "backbone/control.h"
#include "backbone/random.h"

#ifdef ENOKI_AUTODIFF
  #include "backbone/gradient.h"
#endif /* ENOKI_AUTODIFF */

#ifdef BBM_PYTHON
  #include "enoki/python.h"
#endif /* BBM_PYTHON */

/************************************************************************/
/*! \file backbone.h

  \brief Define the available configurations for the enoki backbone

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
  struct floatRGB : public detail::rgbConfig<float, "floatRGB"> {};
  struct doubleRGB : public detail::rgbConfig<double, "doubleRGB"> {};
  //! @}
  
  /**********************************************************************/
  /*! @{ \name Config with packet floating point RGB colors
   **********************************************************************/
  struct floatPacketRGB : public detail::rgbConfig<enoki::Packet<float>, "floatPacketRGB"> {};
  struct doublePacketRGB : public detail::rgbConfig<enoki::Packet<double>, "doublePacketRGB"> {};
  //! @}
  
#ifdef ENOKI_AUTODIFF
  /**********************************************************************/
  /*! @{ \name Config with differentiable floating point RGB colors
   **********************************************************************/
  struct floatDiffRGB : public detail::rgbConfig<enoki::DiffArray<float>, "floatDiffArray"> {};
  struct doubleDiffRGB : public detail::rgbConfig<enoki::DiffArray<double>, "doubleDiffArray"> {};
  //! @}
#endif /* ENOKI_AUTODIFF */

} // end bbm backbone


#endif /* _BBM_ENOKI_BACKBONE_H_ */

