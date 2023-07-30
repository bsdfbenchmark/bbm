#ifndef _BBM_PHONG_WALTER_H_
#define _BBM_PHONG_WALTER_H_

#include "bsdfmodel/scaledmodel.h"
#include "bsdfmodel/microfacet.h"
#include "ndf/phong.h"
#include "maskingshadowing/uncorrelated.h"

/************************************************************************/
/*! \file phongwalter.h

  \brief Implements a microfacet BSDF with a Phong NDF: "Microfacet Models for
  Refraction through Rough Surfaces" [Walter et al. 2007]:
  http://dx.doi.org/10.2312/EGWR/EGSR07/195-206

*************************************************************************/
namespace bbm {

  /**********************************************************************/
  /*! \brief Microfacet BSDF with Phong-based NDF

    \tparam CONF = bbm configuration
    \tparam NAME = model name (default = PhongWalter)

    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, string_literal NAME = "PhongWalter"> requires concepts::config<CONF>
    using phongwalter = scaledmodel<microfacet<ndf::phong<CONF>,
                                               maskingshadowing::uncorrelated<CONF>,
                                               fresnel::cook<CONF>,
                                               microfacet_n::Walter,
                                               NAME>,
                                    bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, phongwalter<config>);
  
} // end bbm namespace

#endif /* _BBM_PHONG_WALTER_H_ */

BBM_EXPORT_BSDFMODEL(bbm::phongwalter);
