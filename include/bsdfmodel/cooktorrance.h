#ifndef _BBM_COOKTORRANCE_H_
#define _BBM_COOKTORRANCE_H_

#include "bsdfmodel/scaledmodel.h"
#include "bsdfmodel/microfacet.h"
#include "ndf/beckmann.h"
#include "maskingshadowing/vgroove.h"

/***********************************************************************/
/*! \file cooktorrance.h
    \brief Implements the Cook-Torrance BSDF Model [SIGGRAPH'82]: 
           https://doi.org/10.1145/357290.357293

************************************************************************/
namespace bbm {

  /*********************************************************************/
  /*! \brief Cook-Torrance microfacet BSDF model.
    
    \tparam CONF = type configuration to use
    \tparam NAME = model name (default = CookTorrance)

    This is a microfacet model set to mimic the original BSDF model
    as proposed by Cook and Torrance.

    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, string_literal NAME = "CookTorrance"> requires concepts::config<CONF> 
  using cooktorrance = scaledmodel<microfacet<ndf::beckmann<CONF, symmetry_v::Isotropic, false>,
                                              maskingshadowing::vgroove<CONF>,
                                              fresnel::cook<CONF>,
                                              microfacet_n::Cook,
                                              NAME>,
                                   bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, cooktorrance<config>);

} // end bbm nanespace

#endif /* _BBM_COOKTORRANCE_H_ */

BBM_EXPORT_BSDFMODEL(bbm::cooktorrance);

