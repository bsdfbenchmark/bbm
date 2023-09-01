#ifndef _BBM_HOLZSCHUCHPACANOWSKI_H_
#define _BBM_HOLZSCHUCHPACANOWSKI_H_

#include "bsdfmodel/microfacet.h"
#include "ndf/epd.h"
#include "maskingshadowing/vanginneken.h"
#include "util/multirange_for.h"

#include "precomputed/holzschuchpacanowski/normalization.h"
#include "precomputed/holzschuchpacanowski/convolution.h"

/************************************************************************/
/*! \file holzschuchpacanowski.h

  \brief The BSDF models from "A two-scale microfacet
  reflectance model combining reflection and diffraction", Holzschuch and
  Pacanowski [2017]: https://doi.org/10.1145/3072959.3073621

  The following models are included:
  + epd: a microfacet BSDF based on the exponential power distribution
  + holzschuchpacanowski: the full two-scale model (specular+difraction)
  + ... diffuse variants.... TODO

*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Exponential Power Distribution microfacet model.

    This microfacet EPD BSDF does _not_ include the specular albedo term (Eq
    25).
  ***********************************************************************/
  template<typename CONF,
           ndf::epd_normalization NORMALIZATION=ndf::epd_normalization::Compute,
           string_literal NAME="EPD"
           > requires concepts::config<CONF>
    using epd = microfacet<ndf::epd<CONF>,
                           maskingshadowing::vanginneken<CONF>,
                           fresnel::complex<CONF>,
                           microfacet_n::Walter,
                           NAME>;

} // end bbm namespace

#endif /* _BBM_HOLZSCHUCHPACANOWSKI_H_ */

BBM_EXPORT_BSDFMODEL(bbm::epd)
