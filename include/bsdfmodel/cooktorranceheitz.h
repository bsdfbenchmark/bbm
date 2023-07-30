#ifndef _BBM_COOKTORRANCEHEITZ_H_
#define _BBM_COOKTORRANCEHEITZ_H_

#include "bsdfmodel/scaledmodel.h"
#include "bsdfmodel/microfacet.h"
#include "ndf/beckmann.h"
#include "maskingshadowing/heightcorrelated.h"

/************************************************************************/
/*! \file cooktorranceheitz.h

  Anisotropic Cook-Torrance BSDF follows: "Understanding the Masking-Shadowing
  Function in Microfacet-Based BRDFs" [Heitz 2014]:
  https://jcgt.org/published/0003/02/03/

*************************************************************************/
namespace bbm {

  /**********************************************************************/
  /*! \brief Microfacet BSDF with Anisotropic Beckmann-based NDF

    \tparam CONF = bbm configuration
    \tparam NAME = model name (default = CookTorranceHeitz)

    Note: the differences with the regular Cook-Torrance BSSF model are:
    + the 1/4 normalization missing from the original Cook-Torrance BSDF
    + using the Beckmann-derived monodirectional shadowing-masking G1
    + height correlated joint shadowing-masking
    + using the anisotropic Beckmann distribition
    
    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, string_literal NAME = "CookTorranceHeitz"> requires concepts::config<CONF>
    using cooktorranceheitz = scaledmodel<microfacet<ndf::beckmann<CONF, symmetry_v::Anisotropic>,
                                                     maskingshadowing::heightcorrelated<CONF>,
                                                     fresnel::cook<CONF>,
                                                     microfacet_n::Walter,
                                                     NAME>,
                                          bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, cooktorranceheitz<config>);
  
} // end bbm namespace

#endif /* _BBM_COOKTORRANCEHEITZ_H_ */

BBM_EXPORT_BSDFMODEL(bbm::cooktorranceheitz);
