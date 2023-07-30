#ifndef _BBM_GGX_H_
#define _BBM_GGX_H_

#include "bsdfmodel/scaledmodel.h"
#include "bsdfmodel/microfacet.h"
#include "ndf/ggx.h"
#include "maskingshadowing/uncorrelated.h"

/************************************************************************/
/*! \file ggx.h

  \brief Implements a microfacet BSDF with a GGX NDF: "Microfacet Models for
  Refraction through Rough Surfaces" [Walter et al. 2007]:
  http://dx.doi.org/10.2312/EGWR/EGSR07/195-206

*************************************************************************/
namespace bbm {

  /**********************************************************************/
  /*! \brief Microfacet BSDF with GGX-based NDF

    \tparam CONF = bbm configuration
    \tparam NAME = model name (default = GGX)

    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, string_literal NAME = "GGX"> requires concepts::config<CONF>
  using ggx = scaledmodel<microfacet<ndf::ggx<CONF, symmetry_v::Isotropic>,
                                     maskingshadowing::uncorrelated<CONF>,
                                     fresnel::cook<CONF>,
                                     microfacet_n::Walter,
                                     NAME>,
                            bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, ggx<config>);

} // end bbm namespace

#endif /* _BBM_GGX_H_ */

BBM_EXPORT_BSDFMODEL(bbm::ggx);

