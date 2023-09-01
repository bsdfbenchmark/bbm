#ifndef _BBM_ASHIKHMIN_SHIRLEY_FULL_H_ 
#define _BBM_ASHIKHMIN_SHIRLEY_FULL_H_

#include "bsdfmodel/ashikhminshirley.h"
#include "bsdfmodel/lambertian.h"

/************************************************************************/
/*! \file ashikhminshirley.h

  \brief Implements the full BSDF (diffuse+specular component) from: "An
  anisotropic phong BRDF model" [Ashikhmin and Shirley 2000]:
  https://doi.org/10.1080/10867651.2000.10487522

************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief the Anisotropic Phong BSSF model by Ashikhmin and Shirley.  This
      implements the combined diffuse+specular component of the model.  The
      combined model is implemented as a single model due to the dependence of
      the diffuse term on the specular fresnel reflectance.

      \tparam CONF = bbm configuration
      \tparam Fresnel = fresnel implementation (requires concepts::Fresnel); default = fresnel::schlick<Config, ior::reflectance<Spectrum>>
      \tparam Symmetry = pass symmetry_v (Default: symmetry_v::Anisotropic)
      \tparam NAME = name of the BSDF model (Default: 'AshikhminShirleyFull')

      Implements: concepts::bsdfmodel
  **********************************************************************/
  template<typename CONF,
           typename Fresnel = fresnel::schlick<CONF, ior::reflectance<Spectrum_t<CONF>>>,
           symmetry_v Symmetry = symmetry_v::Anisotropic,
           string_literal NAME="AshikhminShirleyFull"
           > requires concepts::config<CONF> && concepts::fresnel<Fresnel> && concepts::matching_config<CONF, Fresnel>
    struct ashikhminshirleyfull : public ashikhminshirley<CONF, Fresnel, Symmetry>
  {
    using base = ashikhminshirley<CONF, Fresnel, Symmetry>;
    BBM_BASETYPES(base);
  public:
    BBM_IMPORT_CONFIG( CONF );
    static constexpr string_literal name = NAME;
    BBM_BSDF_FORWARD;
    
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
      // above surface?
      mask &= (vec::z(in) > 0) && (vec::z(out) > 0);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return Spectrum(0);

      // evaluate the specular component
      Spectrum spec = base::eval(in, out, component, unit, mask);
      
      // Quick exit if no diffuse requested
      mask &= is_set(component, bsdf_flag::Diffuse);
      if(bbm::none(mask)) return spec;

      // evaluate the diffuse component
      Value scale = bbm::hprod( Scalar(1.0) - bbm::pow(Scalar(1.0) - 0.5*Vec2d(vec::z(in), vec::z(out)), 5.0) );
      Value normalization = 28.0 / (23.0 * Constants::Pi());

      // Done.
      return bbm::select(mask, (normalization * scale * diffuseReflectance * (Scalar(1.0) - base::fresnelReflectance)) + spec, spec);
    }

    
    /********************************************************************/
    /*! \brief Sample the BSDF given a direction and two random variables.

      \param out = outgoing direction
      \param xi = two random variables stored in a Vec2d
      \param component = which reflectance component to sample
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes.
      \returns A bsdfSample containing the sampled direction and the corresponding pdf.
    *********************************************************************/
    BsdfSample sample(const Vec3d& out, const Vec2d& xi, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      BsdfSample sample = {0,0,bsdf_flag::None};

      // Quick exit if only one component is selected
      if( bbm::none(mask && is_set(component, bsdf_flag::Diffuse)) ) return base::sample(out, xi, component, unit, mask);
      if( bbm::none(mask && is_set(component, bsdf_flag::Specular)) ) return lambertian<Config>().sample(out, xi, component, unit, mask);
      
      // determine weighting
      Value spec_albedo = bbm::hsum( base::fresnelReflectance );
      Value diff_albedo = bbm::hsum( diffuseReflectance ) * (Scalar(1.0) - spec_albedo);
      Value diff_weight = diff_albedo / (diff_albedo + spec_albedo);  
      Value spec_weight = Scalar(1.0) - diff_weight; 

      // sample specular
      Value xi0_spec = bbm::select( spec_weight > Constants::Epsilon(), xi[0] / spec_weight, 0.0);
      BsdfSample spec_sample = base::sample(out, Vec2d(xi0_spec, xi[1]), component, unit, mask);

      // sample diffuse (cf. lambertian)
      Value xi0_diff = bbm::select(diff_weight > Constants::Epsilon(), (xi[0] - spec_weight) / diff_weight, 0.0);
      BsdfSample diff_sample = lambertian<Config>().sample(out, Vec2d(xi0_diff, xi[1]), component, unit, mask);
      
      // combine
      sample = bbm::select( xi[0] <= spec_weight, spec_sample, diff_sample );
      sample.pdf = spec_weight * spec_sample.pdf + diff_weight * diff_sample.pdf;
      
      // Done.
      return sample;
    }
    
    /********************************************************************/
    /*! \brief Compute the pdf given an in and out direction

      \param in = the incoming (sampled) direction
      \param out = the outgoing (given) direction
      \param component = which reflectance component was sampled
      \param unit = unit of computation (ignored)
      \param mask = masking of lanes
      \returns the PDF that the incoming direction would be sampled given the outgoing direction.
    ***********************************************************************/
    Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      // Quick bailout
      if( bbm::none(mask && is_set(component, bsdf_flag::Diffuse)) ) return base::pdf(in, out, component, unit, mask);
      if( bbm::none(mask && is_set(component, bsdf_flag::Specular)) ) return lambertian<Config>().pdf(in, out, component, unit, mask);

      // determine weighting
      Value spec_albedo = bbm::hsum( base::fresnelReflectance );
      Value diff_albedo = bbm::hsum( diffuseReflectance ) * (1.0 - spec_albedo);
      Value diff_weight = bbm::select( diff_albedo > Constants::Epsilon(), diff_albedo / (diff_albedo + spec_albedo), 0);  
      Value spec_weight = 1.0 - diff_weight; 

      // get pdfs
      Value spec_pdf = base::pdf(in, out, component, unit, mask);
      Value diff_pdf = lambertian<Config>().pdf(in, out, component, unit, mask);
      
      // Done.
      return (spec_weight*spec_pdf + diff_weight*diff_pdf);
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
      // check 'out' lies above the horizon
      mask &= (vec::z(out) > 0);

      // Quick exit if mask all negative
      if(bbm::none(mask)) return 0;

      // eval specular reflectance
      Spectrum spec = base::reflectance(out, component, unit, mask);
      
      // Quick exit if no diffuse is requested
      mask &= is_set(component, bsdf_flag::Diffuse);
      if(bbm::none(mask)) return spec;

      // Compute diffuse reflectance
      Spectrum diff = diffuseReflectance * (Scalar(1.0) - base::fresnelReflectance);
      
      // Done
      return bbm::select(mask, diff+spec, spec);
    }

    /////////////////////////
    //! @{ Class Attributes
    /////////////////////////
    diffuse_scale<Spectrum> diffuseReflectance;

    BBM_ATTRIBUTES(diffuseReflectance);
    //! @}

    //! \brief Default constructor
    BBM_DEFAULT_CONSTRUCTOR(ashikhminshirleyfull) {}
  };

  BBM_CHECK_CONCEPT( concepts::bsdfmodel, ashikhminshirleyfull<config> );

} // end bbm namespace


#endif /* _BBM_ASHIKHMIN_SHIRLEY_FULL_H_ */

BBM_EXPORT_BSDFMODEL(bbm::ashikhminshirleyfull)
