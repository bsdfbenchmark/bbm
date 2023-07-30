#ifndef _BBM_COOKTORRANCEWALTER_H_
#define _BBM_COOKTORRANCEWALTER_H_

#include "bsdfmodel/scaledmodel.h"
#include "bsdfmodel/microfacet.h"
#include "ndf/beckmann.h"
#include "maskingshadowing/uncorrelated.h"

/************************************************************************/
/*! \file cooktorrancewalter.h

  \brief Implements a microfacet BSDF with a Beckmann NDF: "Microfacet Models
  for Refraction through Rough Surfaces" [Walter et al. 2007]:
  http://dx.doi.org/10.2312/EGWR/EGSR07/195-206

*************************************************************************/
namespace bbm {

  /**********************************************************************/
  /*! \brief Microfacet BSDF with Beckmann-based NDF

    \tparam CONF = bbm configuration
    \tparam NAME = model name (default = CookTorranceWalter)

    Note: the differences with the regular Cook-Torrance BSSF model are:
    + the 1/4 normalization missing from the original Cook-Torrance BSDF
    + using the Beckmann-derived monodirectional shadowing-masking G1
    + using uncorrelated joint masking and shadowing.
    
    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, string_literal NAME = "CookTorranceWalter"> requires concepts::config<CONF>
  using cooktorrancewalter = scaledmodel<microfacet<ndf::beckmann<CONF, symmetry_v::Isotropic>,
                                                    maskingshadowing::uncorrelated<CONF>,
                                                    fresnel::cook<CONF>,
                                                    microfacet_n::Walter,
                                                    NAME>,
                                           bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, cooktorrancewalter<config>);
  
} // end bbm namespace

#endif /* _BBM_COOKTORRANCEWALTER_H_ */

BBM_EXPORT_BSDFMODEL(bbm::cooktorrancewalter);
