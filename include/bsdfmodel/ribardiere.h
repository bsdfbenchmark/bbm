#ifndef _BBM_RIBARDIERE_H_
#define _BBM_RIBARDIERE_H_

#include "bsdfmodel/scaledmodel.h"
#include "bsdfmodel/microfacet.h"
#include "ndf/studentt.h"
#include "maskingshadowing/uncorrelated.h"

/************************************************************************/
/*! \file std.h

  \brief Implements "STD: Student’s t-Distribution of Slopes for Microfacet
  Based BSDFs" [Ribardiere et al. 2017] https://doi.org/10.1111/cgf.13137

  Implements:: concepts::bsdfmodel
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Microfacet BSDF with Student-T based NDF

    \tparam CONF = bbm configuration
    \tparam NAME = model name (default = Ribadiere)

    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, string_literal NAME = "Ribardiere"> requires concepts::config<CONF>
    using ribardiere = scaledmodel<microfacet<ndf::studentt<CONF, symmetry_v::Isotropic>,
                                              maskingshadowing::uncorrelated<CONF>,
                                              fresnel::cook<CONF>,
                                              microfacet_n::Walter,
                                              NAME>,
                                   bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, ribardiere<config>);

  /**********************************************************************/
  /*! \brief Microfacet BSDF with the Anisotropic Student-T based NDF

    \tparam CONF = bbm configuration
    \tparam NAME = model name (default = RibadiereAnisotripic)

    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF, string_literal NAME = "RibardiereAnisotropic"> requires concepts::config<CONF>
    using ribardiereanisotropic = scaledmodel<microfacet<ndf::studentt<CONF, symmetry_v::Anisotropic>,
                                                         maskingshadowing::uncorrelated<CONF>,
                                                         fresnel::cook<CONF>,
                                                         microfacet_n::Walter,
                                                         NAME>,
                                              bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, ribardiereanisotropic<config>);

  
} // end bbm namespace

#endif /* _BBM_RIBARDIERE_H_ */

BBM_EXPORT_BSDFMODEL(bbm::ribardiere);
BBM_EXPORT_BSDFMODEL(bbm::ribardiereanisotropic);
