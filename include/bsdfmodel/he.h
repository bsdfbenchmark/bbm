#ifndef _BBM_HE_H_
#define _BBM_HE_H_

#include "bbm/bsdfmodel.h"
#include "bbm/ndf_sampler.h"
#include "bsdfmodel/lambertian.h"

/************************************************************************/
/*! \file he.h

  \brief Implements: "A Comprehensive Physical Model for Light Reflection" [He
  1991]: https://doi.org/10.1145/127719.122738

  Follows additonal advice from "Demystifying the He, Torrance, Sillion and
  Greenberg BRDF The BRDF Model" by Pacanowski and Holzschuch:
  https://manao.inria.fr/perso/~pac/hugo/posts/hedemystified/ especially the
  errata:
  https://manao.inria.fr/perso/~pac/hugo/posts/hedemystified/he_errata.pdf

  Additional information was gleaned from the reference implementation of
  Westin et al.: http://www.graphics.cornell.edu/~westin/heBRDF/heBRDF.tar.gz
  with the following bugs fixed in the Vector3 class in Westin et al.'s code:
  z component in 'length' and 'MakeUnitVector', the z component of 'other' in
  operator-, operator+, and the y-component in the solution of the cross
  product.

  Three variants are implemented:
  + 'he' following the original He et al. paper as close as possible.
  + 'hewestin' following Westin et al.'s implementation as cose as possible.
  + 'heholzschuch' following Holzschuch and Pacanowski as close as possible.

  The differences can be summarized by:
  + differences in Eq. 25; He omits an exponential (as noted in the errata).
  + differences in Eq 78; Westin uses tau/lambda*2*PI instead of tau.
  + Westin switches to a explicit rough approximation when g is large.
  + Holzschuch and Pacanowski fix the number of Taylor terms to 10.
  
  Experimentally we see that:
  + The impact of the differences in Shadowing and Masking between with and
    without and the exponential term is 10-15%.
  + For sharp materials (i.e., low roughness and low autocorrelation), all three
    models produce similar results. (within 10%)
  + For rough materials (either high roughness or high autocorrelation), the
    different models differ significantly.
  + 10 Taylor terms (as in Holzschuch and Pacanowski's implementation) is
    insufficient, and can introduce a significant error.

  Westin et al.'s implementation give the most consistent approximation
  between the Taylor expansion and the rough approximation, and is therefore
  likely the most correct.
  
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Masking term scale in Eq 25.

    + WithoutExp = 1.0 (following [He et al. 1991]) + Regular = exp(-
    (tau*cot(theta) / 2sigma0)^2 ) (following the errata, as well as the
    reference implementations of Westin et al. and of Holzschuch and
    Pacanowski)
   **********************************************************************/
  enum struct he_eq25
  {
    WithoutExp,
    Errata,
  };

  /**********************************************************************/
  /*! \brief Eq. 78 variants:

    + Regular: follows He et al. exactly (also in Holzschuch and Pacanowksi).
    + Westin: uses exp(-g - v_xy2 * (tau/lambda)^2 * Pi^2 / m) instead of
      exp(-g -v_xy2 * tau^2 / 4m) in Eq. 78.
  ***********************************************************************/
  enum struct he_eq78
  {
    Regular,
    Westin,
  };
  
  
  /**********************************************************************/
  /*! \brief The directional specular component of the He et al. BSDF model

    \tparam CONF = bbm configuration
    \tparam Fresnel = fresnel implementation (requires concepts::fresnel);
                      default = fresnel::complex
    \tparam EQ25 = determines whether or not to include an extra
                   scaling factor in Eq 25. (Default = he_eq25::WithoutExp)
    \tparam EQ78 = determines which implementation variant to follow for Eq
                   78. ( Default=he_eq78::Regular)
    \tparam NewtonRaphsonSteps = number of Newton Raphson steps to take
                             to compute the apparent roughness (Default=4)
    \tparam TaylorTerms = number of terms in the Taylor expansion for Eq. 78.
                          (Default=40)
    \tparam AdaptiveTaylor = true if the Talylor approximation should be cut
                             of if the change with additional terms is
                             minimal.  If false, all 'TaylorTerms' terms will
                             be computed. (Default = true).
    \tparam RoughApproxThreshold = 'g' threshold at which the use an
                                   approximation instead of the Taylor series.
                                   Both solutions are linearly blended within
                                   g and g+1. (Default=disabled).
    \tparam NAME = model name (default = "He")

    Note: the default settings best approximate the original formulation in He
          et al. 1991
    
    Note: this only implements the directional specular component
    
    Implements: concepts::bsdfmodel
  ***********************************************************************/
  template<typename CONF,
           typename Fresnel=fresnel::complex<CONF, Spectrum_t<CONF>>,
           he_eq25 EQ25=he_eq25::WithoutExp,
           he_eq78 EQ78=he_eq78::Regular,
           size_t NewtonRaphsonSteps=4,
           size_t TaylorTerms=40,
           bool AdaptiveTaylor=true,
           literal RoughApproxThreshold=std::numeric_limits<Scalar_t<CONF>>::max(),
           string_literal NAME="He"> requires concepts::config<CONF> && concepts::fresnel<Fresnel> && concepts::matching_config<CONF, Fresnel>
    class he_base
  {
  public:
    BBM_IMPORT_CONFIG(CONF);
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
    Spectrum eval(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);

      // above surface
      mask &= (vec::z(in) > 0) && (vec::z(out) > 0);

      // Quick exit
      if(bbm::none(mask)) return 0;
      
      // Shadowing
      Value S_term = S(in, out, mask);

      // Geometrical factor
      Value G_term = G(in, out, mask);

      // D dsitribution
      Spectrum D_term = D(in, out, mask);

      // Fresnel
      Value cosThetaHalf = bbm::safe_sqrt( (1 + bbm::dot(in, out)) / 2.0 );
      Spectrum F_term = Fresnel::eval(eta, cosThetaHalf, mask);

      // normalization
      Value normalization = bbm::rcp( Constants::Pi() * spherical::cosTheta(in) * spherical::cosTheta(out) );

      // Done.
      return bbm::select(mask, normalization * F_term * S_term * G_term * D_term, 0);
    }

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
      // matching flag
      mask &= is_set(component, bsdf_flag::Specular);

      // Placeholder
      auto sample = lambertian<Config>().sample(out, xi, bsdf_flag::Diffuse, unit, mask);
      sample.flag = bbm::select(sample.flag == bsdf_flag::Diffuse, BsdfFlag(bsdf_flag::Specular), BsdfFlag(bsdf_flag::None));

      // Done.
      return sample;
    }

   /********************************************************************/
    /*! \brief Compute the pdf given an in and out direction

      \param in = the incoming direction
      \param out = the outgoing direction
      \param component = which reflectance component was sampled
      \param unit = unit of computation
      \param mask = enable/disable lanes.
      \returns the PDF that the outgoing direction would be sampled given the incoming direction.
    *********************************************************************/
    Value pdf(const Vec3d& in, const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t unit=unit_t::Radiance, Mask mask=true) const
    {
      // matching flag
      mask &= is_set(component, bsdf_flag::Specular);

      // Placeholder
      return lambertian<Config>().pdf(in, out, bsdf_flag::Diffuse, unit, mask);
    }


    /*****************************************************************/
    /*! \brief Return the (approximate) hemispherical reflectance of the BSDF

      \param out = the outgoing direction (ignored)
      \param component = which reflectance component to eval
      \param unit = unit of computation
      \param mask = enable/disable lanes
      \returns the approximate hemispherical reflectance of the BSDF for a given incident direction
    ******************************************************************/
    Spectrum reflectance(const Vec3d& out, BsdfFlag component=bsdf_flag::All, unit_t /*unit*/=unit_t::Radiance, Mask mask=true) const
    {
      // specular?
      mask &= is_set(component, bsdf_flag::Specular);

      // check 'out' lies above the horizon
      mask &= (vec::z(out) > 0);

      // Quick exit if mask is all negative
      if(bbm::none(mask)) return 0;      
      
      // Approximate as perfect mirror
      return Fresnel::eval(eta.value(), vec::z(out), mask) / Constants::Pi() * 4.0;
    }

  private:
    /********************************************************************/
    /*! \brief Shadowing term

      \param in = incident direction of light transport
      \param out = exitant direction of light transport
      \param mask = enable/disable lanes
      \returns fraction of surface visible and illuminated.
    *********************************************************************/
    inline constexpr Value S(const Vec3d& in, const Vec3d& out, Mask mask=true) const
    {
      return S1(in, mask) * S1(out, mask);
    }
    
    /********************************************************************/
    /*! \brief Mono-directional shadowing term

      Eqs. 24 and 25 from [He et al. 1991]
      
      \param v = indident/outgoing vector 
      \returns fraction of surface viewed/illuminated

      He et al.'s paper [1991] misses an exponential exp(-(tau*cot/2sigma0)^2)
      term in Eq.  25 (noted in the errata, and by both Westin et al.  and
      Holzschuch and Pacanowski). Set template EQ25 to
      he_eq25::Errata to include this factor.
    *********************************************************************/
    inline constexpr Value S1(const Vec3d& v, Mask mask=true) const
    {
      // Check if smooth
      Mask smooth = (roughness < Constants::Epsilon());

      // Quick exit
      if(bbm::all(smooth)) return 1;
      
      // cot Theta
      Value cotTheta = bbm::rcp( spherical::tanTheta(v) );
      
      // erfc(tau*cot/2sigma0)
      Value scaledCot = autocorrelation * cotTheta / (2.0*roughness);
      Value erfc = 0.5*bbm::erfc( scaledCot );

      // Lambda (Eq. 25)
      Value Lambda = 0.5 * Constants::InvSqrtPi() / scaledCot;
      if constexpr (EQ25 == he_eq25::Errata) Lambda *= bbm::exp(-bbm::pow(scaledCot,2.0));      
      Lambda -= erfc;
      
      // S (Eq. 24)
      Value S = bbm::select(smooth, 1.0, (1.0 - erfc) / (Lambda + 1.0));

      // Done,
      return bbm::select(mask, S, 0.0);
    }

    /********************************************************************/
    /*! \brief Compute the Geometrical factor

      Eq. 76 in [He et al. 1991]

      Note: k_i = -in
            k_r = out
      
      \param in = incident direction of light transport
      \param out = outgoing direction of light transport
      \param mask = enable/disbale lanes
      \returns the geometrical factor
    *********************************************************************/
    inline constexpr Value G(const Vec3d& in, const Vec3d& out, Mask mask=true) const
    {
      // k_i = -in
      // k_r = out
      // Compute (v.v/z(v))^2, with v = k_r - k_i and n=(0,0,1)
      Vec3d v = in + out;
      Value v_scale = bbm::pow(bbm::squared_norm(v) / vec::z(v), 2.0);

      // Omitting i and r subscript, in 'k' and the definition of 's' and 'p':
      // s = k x n / ||k x n||   (1)
      // p = s x k               (2)  
      //
      // Note: ||k x n|| = |sin(theta_k)|   (3)
      // Note: k is normalized
      //
      // ||k_i x k_r||^2 = sin^2(theta_{i,r})       (using (3))
      //                 = 1 - cos^2(theta_{i,r})   (trig)
      //                 = 1 - (k_i . k_r)^2        (trig)
      //
      // a = {i,r} and b = {r,i}
      // s_a . k_b = (k_a x n) . k_b / sin(theta_{k_a})         (using (1) an (3))
      //           = n . (k_b x k_a) / sin(theta_{k_a})         (dot of cross)
      //           = (k_bx*k_ay - k_by*k_ax) / sin(theta_{k_a}  (n=z => z-component of cross)
      //
      // p_a . k_b = (s_a x k_a) . k_b                                          (using (2))
      //           = ((k_a x n) x k_a) . k_b / sin(theta_{k_a})                 (using (1) and (3))
      //           = (n(k_a.k_a) - k_a(k_a.n)) . k_b / sin(theta_{k_a})         (cross of cross)
      //           = ((n.k_b)(k_a.k_a) - (k_a.k_b)(k_a.n)) / sin(theta_{k_a})   (bring in dot)
      //           = (k_bz - (k_a.k_b)k_az) / sin(theta_{k_a})                  (simplify)
      
      Value kixn2 = 1 - vec::z(in)*vec::z(in);
      Value krxn2 = 1 - vec::z(out)*vec::z(out);
      Value kikr = bbm::dot(-in, out);

      Value sikr = (vec::y(out)*vec::x(in) - vec::x(out)*vec::y(in)); // / kixn
      Value srki = (vec::y(in)*vec::x(out) - vec::x(in)*vec::y(out)); // / krxn
      Value pikr = vec::z(out) + kikr*vec::z(in); // / kixn
      Value prki = vec::z(in) + kikr*vec::z(out); // / krxn * -1
      
      // Eq. 76:  (v.v)/z(v)^2 * ((s_r.k_i)^2 + (p_r.k_i)^2)((s_i.k_r)^2 + (p_i.k_r)^2) / |k_i x k_r|^4
      Value denom = bbm::pow(1.0 - kikr*kikr, 2.0);
      Value nom = (bbm::pow(sikr,2.0) + bbm::pow(pikr,2.0)) * (bbm::pow(srki,2.0) + bbm::pow(prki,2.0)) / (krxn2*kixn2);
      Value g = bbm::select(denom > Constants::Epsilon(), v_scale * nom / denom, 1.0);

      // Done.
      return bbm::select(mask, g, 0.0);
    }

    /********************************************************************/
    /*! \brief Compute the effective surface roughness

      Eq. 80 from [He et al. 1991]. This equation is solved with a
      Newton-Raphson root finding.
      
      \param in = incident direction of light transport
      \param out = outgoing direction of light transport
      \param mask = enable/disbale lanes
      \returns the apparent roughness
    *********************************************************************/
    inline constexpr Value sigma(const Vec3d& in, const Vec3d& out, Mask mask=true) const
    {
      // check if surface is smooth
      mask &= (roughness > Constants::Epsilon());

      // Quick exit if all masks are false
      if(bbm::none(mask)) return 0;

      // tan
      Value tanTheta_i = spherical::tanTheta(in);
      Value tanTheta_o = spherical::tanTheta(out);

      // Compute K (Eq. 82-83) 
      auto K = [&](const Value& tanTheta) { return tanTheta * bbm::erfc( autocorrelation / (2 * roughness * tanTheta) ); };
      Value Ki = bbm::select( tanTheta_i > Constants::Epsilon(), K(tanTheta_i), 0 );
      Value Ko = bbm::select( tanTheta_o > Constants::Epsilon(), K(tanTheta_o), 0 );

      // Set initial solution
      Value f0 = bbm::rsqrt(Constants::Pi(8.0)) * (Ki + Ko);
      Value x = bbm::select(f0 <= 1.0, f0, bbm::safe_sqrt(2.0*bbm::log(f0)));

      // Newton-Raphson root finding (emperical observation: tends to converge less than 4 steps)
      for(size_t step=0; step < NewtonRaphsonSteps; ++step)
      {
        // Eval function and derivative
        Value expn = bbm::exp(0.5*x*x);
        Value eval = x*expn - f0;
        Value grad = (1+x*x)*expn;

        // Update root
        x -= bbm::select(grad > Constants::Epsilon(), eval / grad, 0);
      }

      // Done.
      return bbm::select(mask, roughness / bbm::safe_sqrt(1 + x*x), 0.0);
    }
    
    /********************************************************************/
    /*! \brief Distribution Function

      Eq. 78 & 79

      Note Westin et al. uses exp(-g - v_xy2 * (tau/lambda)^2 * Pi^2 / m)
      instead of exp(-g - v_xy2 * tau^2 / (4m)). This behavior can be set with
      the EQ78 template parameter (he_eq78::Westin).
    *********************************************************************/
    inline constexpr Spectrum D(const Vec3d& in, const Vec3d& out, Mask mask=true) const
    {
      // Compute v_xy^2 (Eq. 84)
      Value v_xy2 = bbm::squared_norm( vec::xy(in) + vec::xy(out) );

      // Compute g (Eq. 79)
      auto g = bbm::pow( Constants::Pi(2) * sigma(in,out,mask) * (spherical::cosTheta(in) + spherical::cosTheta(out)) / Config::wavelength(), 2.0);
      
      // Compute exp(-g) normalization term: pi^2 * tau^2 / 4wavelength^2  (Eq. 78)
      Value tau2 = bbm::pow(autocorrelation, 2.0);
      auto normalization = Constants::Pi2(0.25) * tau2 / bbm::pow(Config::wavelength(), 2.0);

      // Determine exp_base for (Eq. 78)
      Spectrum exp_base = v_xy2 * tau2 / 4;
      if constexpr (EQ78 == he_eq78::Westin) exp_base *= Constants::Pi2(4) / bbm::pow(Config::wavelength(), 2.0);

      // Rough Approximation (Beckmann '63, Section 5.3, Eq. 47, p 87)  (excluding normalization)
      Mask approx = mask && (bbm::hmin(g) > Scalar(RoughApproxThreshold));
      Spectrum roughApprox = 0;
      Value weight = 0;
      if(bbm::any(approx))
      {
        roughApprox = bbm::exp( -exp_base / g ) / g;

        // Linearily blend with the Taylor Series when g =
        // [RoughApproxThreshold, RoughApproxThreshold+1] to avoid
        // discontinuities.
        weight = bbm::clamp(bbm::hmin(g) - Scalar(RoughApproxThreshold), 0, 1);
      }
      
      // Sum term (Eq. 78)   (excluding normalization)
      Spectrum sum = 0;
      Spectrum gm = 1;
      Spectrum last=-1, term=0;
      Mask converged = !mask || ((bbm::hmin(g)-1) > Scalar(RoughApproxThreshold));
      for(size_t m=1; m <= TaylorTerms && bbm::any(!converged); ++m)
      {
        // copy previous 'term'
        last = term;

        // compute new term and add to sum.
        gm *= g / m;
        term = bbm::exp(-g - exp_base / m) * gm / m;
        sum += bbm::select(converged, 0, term);
        
        // Did the Taylor Expansion Converge?  We follow Westin et al. and
        // check that the magnitude of the terms gets small, and are
        // diminishing (the series tends to increase first and then decrease).
        if constexpr (AdaptiveTaylor) converged |= (bbm::hmin(term) < Constants::Epsilon()) && (bbm::hmin(term) < bbm::hmin(last));
      }

      // Blend Taylor Approximation and Rough Approximation
      auto result = bbm::lerp(sum, roughApprox, weight);
      
      // Done.
      return bbm::select(mask, normalization * result, 0);
    }
    
  public:
    ///////////////////////////////
    //! @{ \name Class Attributes
    ///////////////////////////////
    bsdf_parameter<Value, bsdf_attr::SpecularParameter, 0.18> roughness;       // sigma
    bsdf_parameter<Value, bsdf_attr::SpecularParameter, 3.0> autocorrelation;  // tau
    fresnel_parameter<typename std::decay_t<Fresnel>::parameter_type> eta;

    BBM_ATTRIBUTES(roughness, autocorrelation, eta);
    //! @}

    //! \brief Default Constructor
    BBM_DEFAULT_CONSTRUCTOR(he_base) {}
  };

  BBM_CHECK_CONCEPT(concepts::bsdfmodel, he_base<config>);

  /**********************************************************************/
  /*! @{ \name He BSDF Variants with data-driven importance sampling
   **********************************************************************/
  template<typename CONF, string_literal NAME="He"> requires concepts::config<CONF>
    using he = ndf_sampler<he_base<CONF, fresnel::complex<CONF, Spectrum_t<CONF>>, he_eq25::WithoutExp, he_eq78::Regular, 4, 64, true, 18>, 90, 1, NAME>;
  
  template<typename CONF, string_literal NAME="HeWestin"> requires concepts::config<CONF>
    using hewestin = ndf_sampler<he_base<CONF, fresnel::complex<CONF, Spectrum_t<CONF>>, he_eq25::Errata, he_eq78::Westin, 4, 64, true, 18>, 90, 1, NAME>;

  template<typename CONF, string_literal NAME="HeHolzschuch"> requires concepts::config<CONF>
    using heholzschuch = ndf_sampler<he_base<CONF, fresnel::complex<CONF, Spectrum_t<CONF>>, he_eq25::Errata, he_eq78::Regular, 4, 10, false>, 90, 1, NAME>;
  //! @}
  
} // end bbm namespace

#endif /* _BBM_HE_H_ */

BBM_EXPORT_BSDFMODEL(bbm::he)
BBM_EXPORT_BSDFMODEL(bbm::hewestin)
BBM_EXPORT_BSDFMODEL(bbm::heholzschuch)
