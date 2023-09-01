#ifndef _BBM_NGAN_H_
#define _BBM_NGAN_H_

#include "bsdfmodel/aggregatemodel.h"
#include "bsdfmodel/lambertian.h"
#include "bsdfmodel/ward.h"
#include "bsdfmodel/wardduer.h"
#include "bsdfmodel/phong.h"
#include "bsdfmodel/lafortune.h"
#include "bsdfmodel/cooktorrance.h"
#include "bsdfmodel/ashikhminshirley.h"

/************************************************************************/
/*! \file ngan.h

  \brief All the BSDF variants used in "Experimental Analysis of BRDF Model"
  [Ngan et al. 2005]: https://dl.acm.org/doi/10.5555/2383654.2383671

  Note: these are only the specular component of the models. Use
  aggregatemodel to combine them with a lambertian model.
  
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Ngan's Ward: standard Ward with isotropic roughness. This can be
      achieved with a simple alias.
  ***********************************************************************/
  template<typename CONF, string_literal NAME="NganWard"> requires concepts::config<CONF>
    using nganward = ward<CONF, symmetry_v::Isotropic, NAME>;

  /**********************************************************************/
  /*! \brief Ngan's Ward-Duer: stanard Ward-Duer with isotropic
      roughness. This can be achieved with a simple alias.
  ***********************************************************************/
  template<typename CONF, string_literal NAME="NganWardDuer"> requires concepts::config<CONF>
    using nganwardduer = wardduer<CONF, symmetry_v::Isotropic, NAME>;

  /**********************************************************************/
  /*! \brief Ngan's Blinn-Phong: same as modifed phong; a simple alias suffices.
  ***********************************************************************/
  template<typename CONF, string_literal NAME="NganBlinnPhong"> requires concepts::config<CONF>
  using nganblinnphong = phong<CONF, NAME>;

  /**********************************************************************/
  /*! \brief Ngan's Lafortune: standard Lafortune times an additional
    normalization factor:

    \f$ \frac{ (n+2) }{ \2pi * (\max(Cz, Cxy))^n } \f$

    This is achieved with a wrapper class.
  ***********************************************************************/
  template<typename CONF, string_literal NAME="NganLafortune"> requires concepts::config<CONF>
    class nganlafortune : public lafortune<CONF, symmetry_v::Isotropic>
  {
    using base = lafortune<CONF, symmetry_v::Isotropic>;
    BBM_BASETYPES(base);
  public:
    BBM_IMPORT_CONFIG( CONF );
    static constexpr string_literal name = NAME;
    BBM_BSDF_FORWARD;
    
    // pass through sample and pdf
    using base::base;
    using base::sample;
    using base::pdf;

    /********************************************************************/
    /*! \brief Evaluate the BSDF for a given in and out direction

      \param in = incident direction
      \param out = outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes (e.g., for Packet eval)
      \returns Evaluation of the BSDF per spectrum.
    *********************************************************************/
    Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);

      // above surface?
      mask &= (vec::z(in) > 0) && (vec::z(out) > 0);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return Spectrum(0);

      // Eval Lafortune
      Spectrum result = base::eval(in, out, component, unit, mask);

      // Scale by Ngan normalization
      result *= (base::sharpness + 2.0) * Constants::InvPi(0.5) / bbm::pow( bbm::max( bbm::squared_norm(base::Cz), bbm::squared_norm(base::Cxy) ), base::sharpness * Scalar(0.5));

      // Done.
      return bbm::select(mask, result, 0);
    }

    /*******************************************************************/
    /*! \brief Return the (approximate) reflectance of the BSDF

      \param out = the outgoing direction
      \param component = which reflectance component to eval
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes
      \returns the approximate reflectance of the BSDF for a given direction
    ********************************************************************/
    Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);

      // Quick exit if mask all negative
      if(bbm::none(mask)) return Spectrum(0);

      // Eval reflectance Lafortune
      Spectrum result = base::reflectance(out, component, unit, mask);

      // Scale by Ngan normalization
      result *= (base::sharpness + 2.0) * Constants::InvPi(0.5) / bbm::pow(bbm::max( bbm::squared_norm(base::Cz), bbm::squared_norm(base::Cxy) ), base::sharpness * Scalar(0.5));

      // Done.
      return bbm::select(mask, result, 0);
    }
    
    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(nganlafortune) {}
  };

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, nganlafortune<config>);


  /**********************************************************************/
  /*! \brief Ngan's Cook-Torrance is normalized by pi, is isotropic, and has
      vgroove masking and shadowing, schlick fresnel, F0 reflectance, and
      backman distribution

      This is achieved by a defining a custom microfacet model.
  ***********************************************************************/
  template<typename CONF, string_literal NAME="NganCookTorrance"> requires concepts::config<CONF>
    using ngancooktorrance = scaledmodel<microfacet<ndf::beckmann<CONF, symmetry_v::Isotropic>,
                                                    maskingshadowing::vgroove<CONF>,
                                                    fresnel::schlick<CONF>,
                                                    microfacet_n::Cook,
                                                    NAME>,
                                         bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, ngancooktorrance<config>);


  /**********************************************************************/
  /*! \brief Ngan's Ashkihmin-Shirley model; does not include
      Ashikhmin-Shirley's diffuse component, and it is isotropic.  THis can be
      achieved with a simple alias.
  ***********************************************************************/
  template<typename CONF, string_literal NAME="NganAshikhminShirley"> requires concepts::config<CONF>
    using nganashikhminshirley = scaledmodel<ashikhminshirley<CONF, fresnel::schlick<CONF, ior::reflectance<Value_t<CONF>>>, symmetry_v::Isotropic, NAME>, bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, nganashikhminshirley<config>);

  /**********************************************************************/
  /*! \brief Ngan's He et al.'s BSDF likely uses the Westin et al. formulation
      but with non-complex eta.
    *********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    using nganhe = scaledmodel<ndf_sampler<he_base<CONF, fresnel::cook<CONF>, he_eq25::Errata, he_eq78::Westin, 4, 64, true, 18>, 90, 1, "NganHe">, bsdf_attr::SpecularScale>;

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, nganhe<config>);
  
} // end bbm namespace

#endif /* _BBM_NGAN_H_ */


BBM_EXPORT_BSDFMODEL(bbm::nganward)
BBM_EXPORT_BSDFMODEL(bbm::nganwardduer)
BBM_EXPORT_BSDFMODEL(bbm::nganblinnphong)
BBM_EXPORT_BSDFMODEL(bbm::nganlafortune)
BBM_EXPORT_BSDFMODEL(bbm::ngancooktorrance)
BBM_EXPORT_BSDFMODEL(bbm::nganashikhminshirley)
BBM_EXPORT_BSDFMODEL(bbm::nganhe)

