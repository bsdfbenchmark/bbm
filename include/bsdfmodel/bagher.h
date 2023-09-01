#ifndef _BBM_BAGHER_H_
#define _BBM_BAGHER_H_

#include "bsdfmodel/microfacet.h"
#include "bsdfmodel/scaledmodel.h"
#include "ndf/sgd.h"
#include "maskingshadowing/uncorrelated.h"

/************************************************************************/
/*! \file bagher.h

  \brief Implements the microfacet BSDF with Shifted Gamma Distribution:

  M. M. Bagher, C. Soler, and N. Holzschuch "Accurate fitting of measured
  reflectances using a Shifted Gamma micro-facet distribution", Computer
  Graphics Forum 31, 4, 2012: https://doi.org/10.1111/j.1467-8659.2012.03147.x

*************************************************************************/

namespace bbm {

  namespace ior {
    //! @{ \name Specialized 2d reflectance type for Bagher et al.'s modified Schlick's approxiation.
    template<typename T> struct bagher_reflectance_tag { using type = vec2d<T>; };
    template<typename T> using bagher_reflectance = attribute<bagher_reflectance_tag<T>>;
    //! @}
  } // end ior namespace
  
  /**********************************************************************/
  /*! \brief Define a specialized bsdf_parameter for the 2d Bagher et al.'s reflectance.
   **********************************************************************/
  template<typename T, bsdf_attr Flag> struct bsdf_fresnel_properties<ior::bagher_reflectance<T>, Flag> : bsdf_properties<ior::bagher_reflectance<T>, Flag, std::array{1.0,0.0}, 1.0, std::array{0.0, -1.0}> {};

  namespace fresnel {
    /********************************************************************/
    /*! \brief Bagher at al.'s modified Schlick's Fresnel approximation
     ********************************************************************/
    template<typename CONF> requires concepts::config<CONF>
      struct bagher
    {
      BBM_IMPORT_CONFIG( CONF );

      //! \brief uses specialized bagher_reflectance
      using parameter_type = ior::bagher_reflectance<Spectrum>;

      //! \brief Evaluate Bagher's modified Schlick Fresnel
      static constexpr Spectrum eval(const parameter_type& F, const Value& cosTheta, Mask mask=true)
      {
        return bbm::select(mask, fresnel::schlick<Config,ior::reflectance<Spectrum>>::eval(F[0], cosTheta, mask) - (F[1] * cosTheta), 0);
      }
    };

    BBM_CHECK_CONCEPT( concepts::fresnel, bagher<config> );
  } // end fresnel namespace
  
  /**********************************************************************/
  /*! \brief Bagher et al.'s Microfacet BSDF with Shifted Gamma Distribution

    \tparam CONF = bbm configuration
    \tparam NAME = model name (default = Bagher)
   **********************************************************************/
  template<typename CONF, string_literal NAME = "Bagher"> requires concepts::config<CONF>
    using bagher = scaledmodel<microfacet<ndf::sgd<CONF>,
                                          maskingshadowing::uncorrelated<CONF>,
                                          fresnel::bagher<CONF>,
                                          microfacet_n::Cook,
                                          NAME>,
                               bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, bagher<config> );

} // end bbm namespace

#endif /* _BBM_BAGHER_H_ */

BBM_EXPORT_BSDFMODEL(bbm::bagher)
