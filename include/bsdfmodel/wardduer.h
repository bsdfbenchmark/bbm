#ifndef _BBM_WARD_DUER_H_
#define _BBM_WARD_DUER_H_

#include "bsdfmodel/ward.h"

/************************************************************************/
/*! \file wardduer.h

  \brief Implements: "An Improved Normalization for the Ward Reflectance
  Model" [Arne Duer 2006]: https://doi.org/10.1080/2151237X.2006.10129215

*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief The anisotropic Ward-Duer BSDF model.

    \tparam CONF = bbm configuration
    \tparam Symmetry = isotropic or anisotropic (default: symmetry_v::anisotropic)
    \tparam NAME = name of the BSDF model (Default: 'WardDuer')

    This model differs from the regular Ward BSDF model in the normalization
    (sqrt(cos(theta(in)) * cos(theta((out))) for Ward versus
    cos(theta(in))*cos(theta(out)) for Ward-Duer).
    
    Implements: concepts::bsdfmodel
   **********************************************************************/
  template<typename CONF, symmetry_v Symmetry = symmetry_v::Anisotropic, string_literal NAME="WardDuer"> requires concepts::config<CONF>
    class wardduer : public ward<CONF, Symmetry>
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
      Value normalizationFactor = Constants::Pi(4) * vec::x(r) * vec::y(r) * ( vec::z(in) * vec::z(out) );
      Value exponent = bbm::squared_norm(vec::xy(H) / r) / bbm::pow(vec::z(H), 2);

      Spectrum result = base::albedo;
      result *= bbm::exp(-exponent) / normalizationFactor;

      // Done.
      return bbm::select(mask, result, 0);
    }

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(wardduer) {}
  };

  BBM_CHECK_CONCEPT( concepts::bsdfmodel, wardduer<config>);
  
} // end bbm namespace

#endif /* _BBM_WARD_DUER_H_ */

BBM_EXPORT_BSDFMODEL(bbm::wardduer)
