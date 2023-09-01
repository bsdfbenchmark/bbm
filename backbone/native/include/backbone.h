#ifndef _BBM_NATIVE_BACKBONE_H_
#define _BBM_NATIVE_BACKBONE_H_

#include "util/string_literal.h"

#include "backbone/array.h"
#include "backbone/complex.h"
#include "backbone/vec.h"
#include "backbone/color.h"
#include "backbone/type_traits.h"
#include "backbone/control.h"
#include "backbone/math.h"
#include "backbone/horizontal.h"
#include "backbone/random.h"
#include "backbone/python.h"
#include "backbone/stringconvert.h"

/************************************************************************/
/*! \file backbone.h

  \brief Define the available configurations for the backbone

*************************************************************************/

namespace bbm {

  /*** Implementation detail ***/
  namespace detail {
    template<typename VALUE, string_literal NAME, typename CONF>
      struct rgbConfig
    {
      static constexpr string_literal name = NAME;
      using Config = CONF;
      using Value = VALUE;
      using Spectrum = backbone::color<Value>;
      static Spectrum wavelength(void) { return {0.645, 0.526, 0.444}; } // in micron
    };
  }
  
  /*** Default configurations ***/
  struct floatRGB : public detail::rgbConfig<float, "floatRGB", floatRGB> {};
  struct doubleRGB : public detail::rgbConfig<double, "doubleRGB", doubleRGB> {};

} // end bbm namespace

#endif /* _BBM_NATIVE_BACKBONE_H_ */
