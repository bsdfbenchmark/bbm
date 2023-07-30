#ifndef _BBM_WARD_DUER_GEISLER_MORODER_H_
#define _BBM_WARD_DUER_GEISLER_MORODER_H_

#include "bsdfmodel/ward.h"

/************************************************************************/
/*! \file wardduergeislermoroder.h

  \brief Implements: "A New Ward BRDF Model with Bounded Albedo",
  [Geisler-Moroder and Duer 2010]:
  https://doi.org/10.1111/j.1467-8659.2010.01735.x

*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief The anisotropic Ward-Duer-Geisler-Moroder BSDF model.

    \tparam CONF = bbm configuration
    \tparam Symmetry = isotropic or anisotropic (default: symmetry_v::anisotropic)
    \tparam NAME = name of the BSDF model (Default: 'WardDuerGeislerMoroder')

    This model differs from the Ward-Duer BRDF model in that it includes a
    scaling term to preserve energy conservation at grazing angles.
    
    Implements: concepts::bsdfmodel
   **********************************************************************/
  template<typename CONF, symmetry_v Symmetry = symmetry_v::Anisotropic, string_literal NAME="WardDuerGeislerMoroder"> requires concepts::config<CONF>
    class wardduergeislermoroder : public ward<CONF, Symmetry>
  {
    using base = ward<CONF, Symmetry>;
    BBM_BASETYPES(base);
  public:
    BBM_IMPORT_CONFIG( CONF );
    static constexpr string_literal name = NAME;
    BBM_BSDF_FORWARD;
    
    // Copy sample, pdf, and reflectance unchanged from ward
    using base::sample;
    using base::pdf;
    using base::reflectance;
    
    /********************************************************************/
    /*! \brief Evaluate the BSDF for a given in and out direction

      \param in = incident direction
      \param out = outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes (e.g., for Packet eval)
      \returns Evaluation of the BSDF per spectrum.
    *********************************************************************/
    Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);

      // above surface?
      mask &= (vec::z(in) >= 0) && (vec::z(out) >= 0);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return Spectrum(0);

      // Evaluate BRDF
      Vec2d r(base::roughness);    // copy anisotropic | map isotropic to Vec2d.      
      Vec3d H = in + out;
      Value zH2 = vec::z(H)*vec::z(H);
      Value normalizationFactor = Constants::Pi(4) * vec::x(r) * vec::y(r) * bbm::pow(zH2, 2.0) / bbm::dot(H, H);
      auto exponent = bbm::squared_norm(vec::xy(H) / r) / zH2;

      auto result = base::albedo;
      result *= bbm::exp(-exponent) / normalizationFactor;

      // Done.
      return bbm::select(mask, result, 0);
    }

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(wardduergeislermoroder) {}
  };

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, wardduergeislermoroder<config>);
  
} // end bbm namespace

#endif /* _BBM_WARD_DUER_GEISLER_MORODER_H_ */

BBM_EXPORT_BSDFMODEL(bbm::wardduergeislermoroder);
