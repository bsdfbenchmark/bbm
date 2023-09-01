#ifndef _BBM_NDF_IMPORTANCE_SAMPLER_H_
#define _BBM_NDF_IMPORTANCE_SAMPLER_H_

#include <map>

#include "concepts/bsdfmodel.h"
#include "util/reflection.h"
#include "util/reference.h"
#include "ndf/sampler.h"

/************************************************************************/
/*! \file ndf_sampler.h

  \brief Replaces a(n isotrpic) BSDF model's importance sampling with a
  data-driven NDF based sampler.

  Assumes the BSDF model is isotropic.

************************************************************************/

namespace bbm {

  template<typename BSDFMODEL, size_t samplesTheta=90, size_t samplesPhi=1, string_literal NAME=BSDFMODEL::name> requires concepts::bsdfmodel<BSDFMODEL>
    class ndf_sampler : public BSDFMODEL
  {
    BBM_BASETYPES( BSDFMODEL );

    /********************************************************************/
    /*! \brief Helper Class: Backscatter NDF passthrough

      Wraps the BSDF model's eval function in an NDF eval; other NDF methods
      are unevaluable.  This is used to construct an ndf::sampler
    ********************************************************************/
    struct backscatter
    {
      BBM_BASETYPES(void); // Avoid clash with the base_type of ndf_sampler
      BBM_IMPORT_CONFIG( BSDFMODEL );
      static constexpr string_literal name = "backscatter_" + NAME;
      
      backscatter(void) {}
      backscatter(const BSDFMODEL* src, bsdf_flag c, unit_t u) : component(c), unit(u), model(src) {}
      
      Value eval(const Vec3d& halfway, Mask mask=true) const { return bbm::hsum(model->eval(halfway, halfway, component, unit, mask)); }
      Vec3d sample(const Vec3d& view, const Vec2d& xi, Mask mask=true) const; // unevaluable
      Value pdf(const Vec3d& view, const Vec3d& m, Mask mask=true) const; // unevaluable
      Value G1(const Vec3d& v, const Vec3d& m, Mask mask=true) const; // unevaluable
      
      bsdf_flag component;          //< component to sample
      unit_t unit;                  //< unit to sample
      const BSDFMODEL* model;       //< pointer to BSDF model to sample
      BBM_ATTRIBUTES( reflection::attributes(*model) );
    };

    BBM_CHECK_RAW_CONCEPT(concepts::ndf, backscatter);
    
  public:
    BBM_IMPORT_CONFIG( BSDFMODEL );
    static constexpr string_literal name = NAME;
    BBM_BSDF_FORWARD;
    
    //! forward to parent constructor, with empty _sampler (due to reference to *this)
    template<typename... Ts> requires std::constructible_from<BSDFMODEL, Ts...>
      ndf_sampler(Ts&&... ts) : BSDFMODEL(std::forward<Ts>(ts)...), _samplers() {}
    
    // Passthrough: constructors, eval and reflectance
    using BSDFMODEL::eval;
    using BSDFMODEL::reflectance;
       
    /********************************************************************/
    /*! \brief Sample the diffuse BSDF given a direction and two random variables.

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

      // check for valid xi
      mask &= (xi[0] >= 0) && (xi[1] >= 0) && (xi[0] <= 1) && (xi[1] <= 1);

      // check out lies above the horizon
      mask &= (vec::z(out) > 0);
      
      // Quick exit if mask all negative
      if(bbm::none(mask)) return sample;

      // Sample the correct ndf based on (component,unit)
      Vec3d halfway;

      for(auto u : bbm::reflection::enum_v<unit_t>)
        for(auto c : bbm::reflection::enum_v<bsdf_flag>)
        {
          auto sample_mask = mask && bbm::eq(component, c) && bbm::eq(unit, u);
          if(bbm::any(sample_mask))
          {
            // Create sampler if it does not exist for the given component/unit combination
            auto [itr,init] = _samplers.try_emplace(std::pair(c,u), ndf::sampler<backscatter, samplesTheta, samplesPhi>(this, c, u));

            // update sampled halfway
            halfway = bbm::select(sample_mask, itr->second.sample(out, xi, sample_mask), halfway);
          }
        }

      // haflway -> direction
      sample.direction = bbm::select(mask, reflect(out, halfway), 0);
      sample.pdf = pdf(sample.direction, out, component, unit, mask);
      sample.flag = bbm::select(mask, component, BsdfFlag(bsdf_flag::None));

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
      // check out and in lie above the horizon
      mask &= (vec::z(out) > 0) && (vec::z(in) > 0);
      
      // Quick exit if mask all negative
      if(bbm::none(mask)) return 0;

      // Compute PDF for the correct ndf based on (component,unit)
      Vec3d h = halfway(in, out);
      Value pdf(0);

      for(auto u : bbm::reflection::enum_v<unit_t>)
        for(auto c : bbm::reflection::enum_v<bsdf_flag>)
        {
          auto pdf_mask = mask && bbm::eq(component, c) && bbm::eq(unit, u);
          if(bbm::any(pdf_mask))
          {
            // Create sampler if it does not exist for the given component/unit combination
            auto [itr,init] = _samplers.try_emplace(std::pair(c,u), ndf::sampler<backscatter, samplesTheta, samplesPhi>(this, c, u));

            // update pdf
            pdf = bbm::select(pdf_mask, itr->second.pdf(out, h, pdf_mask), pdf);
          }
        }
      
      // Done; pdf = pdfh) / (4 out . h)
      return bbm::select(mask, pdf / bbm::abs(4.0 * bbm::dot(out, h)), 0);
    }
    
  private:
    ///////////////////////////////
    //! @{ \name Class Attributes
    ///////////////////////////////
    mutable std::map< std::pair<bsdf_flag, unit_t>, ndf::sampler<backscatter, samplesTheta, samplesPhi> > _samplers;
  };

} // end bbm namespace

#endif /* _BBM_NDF_IMPORTANCE_SAMPLER_H_ */
