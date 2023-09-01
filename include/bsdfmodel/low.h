#ifndef _BBM_LOW_FITMODELS_H_
#define _BBM_LOW_FITMODELS_H_

#include "bbm/bsdfmodel.h"
#include "bsdfmodel/ashikhminshirley.h"
#include "bsdfmodel/lambertian.h"
#include "bsdfmodel/lowmicrofacet.h"

/************************************************************************/
/*! \file low.h

  \brief The bsdfmodels used for the BSDF paramter fits in the supplemental
  material of: "BRDF models for accurate and efficient rendering of glossy
  surfaces" [Low 2012]: https://doi.org/10.1145/2077341.2077350

**************************************************************************/
namespace bbm {

  /**********************************************************************/
  /*! \brief Low et al.'s version of the specular component of the
      Ashikhmin-Shirley BSDF: uses the Cook-Torrance Fresnel component with
      index of refraction.
  ***********************************************************************/
  template<typename CONF, string_literal NAME="LowAshikhminShirley"> requires concepts::config<CONF>
    using lowashikhminshirley = scaledmodel<ashikhminshirley<CONF, fresnel::cook<CONF, ior::ior<Value_t<CONF>>>, symmetry_v::Isotropic, NAME>, bsdf_attr::SpecularScale>;
  
  /**********************************************************************/
  /*! \brief Low et al.'s version of the specular component of the
      Cook-Torrance BSDF; this directly mirrors the original Cook-Torrance
      implementation.
  ***********************************************************************/
  template<typename CONF, string_literal NAME="LowCookTorrance"> requires concepts::config<CONF>
    using lowcooktorrance = cooktorrance<CONF, "LowCookTorrance">;
  
  /**********************************************************************/
  /*! \brief Shorthand for the Low BSDF model compatible with the fits listed
      in their supplemental material which differs from the model in the paper
      by using a different normalization factor.
  ***********************************************************************/
  template<typename CONF, string_literal NAME="LowMicrofacetFit"> requires concepts::config<CONF>
    using lowmicrofacetfit = lowmicrofacet<CONF, microfacet_n::Cook, NAME>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, lowmicrofacetfit<config>);

} // end bbm namespace

#endif /* _BBM_LOW_FITMODELS_H_ */

BBM_EXPORT_BSDFMODEL(bbm::lowashikhminshirley)
BBM_EXPORT_BSDFMODEL(bbm::lowcooktorrance)
BBM_EXPORT_BSDFMODEL(bbm::lowmicrofacetfit)
