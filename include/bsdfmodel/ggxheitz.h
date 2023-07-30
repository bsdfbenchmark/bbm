#ifndef _BBM_GGX_HEITZ_H_
#define _BBM_GGX_HEITZ_H_

#include "bsdfmodel/scaledmodel.h"
#include "bsdfmodel/microfacet.h"
#include "ndf/ggx.h"
#include "maskingshadowing/heightcorrelated.h"

/************************************************************************/
/*! \file ggx.h

  \brief Implements the anisotropic GGX BSDF from: "Understanding the
  Masking-Shadowing Function in Microfacet-Based BRDFs" [Heitz 2014]:
  https://jcgt.org/published/0003/02/03/
  
*************************************************************************/
namespace bbm {

  /**********************************************************************/
  /*! \brief Microfacet BSDF with anisotropic GGX-based NDF with heigh
      correlated shadowing and masking.

    \tparam CONF = bbm configuration
    \tparam NAME = model name (default = GGXHeitz)

    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, string_literal NAME = "GGXHeitz"> requires concepts::config<CONF>
  using ggxheitz = scaledmodel<microfacet<ndf::ggx<CONF, symmetry_v::Anisotropic>,
                                          maskingshadowing::heightcorrelated<CONF>,
                                          fresnel::cook<CONF>,
                                          microfacet_n::Walter,
                                          NAME>,
                               bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, ggxheitz<config>);

  
} // end bbm namespace

#endif /* _BBM_GGX_HEITZ_H_ */

BBM_EXPORT_BSDFMODEL(bbm::ggxheitz);
