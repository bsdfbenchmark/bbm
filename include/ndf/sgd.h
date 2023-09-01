#ifndef _BBM_SGD_NDF_H_
#define _BBM_SGD_NDF_H_

#include "bbm/ndf.h"
#include "ndf/ggx.h"

/************************************************************************/
/*! \file sgd.h

  \brief Implements the Shifted Gamma Distribution:

  M. M. Bagher, C. Soler, and N. Holzschuch "Accurate fitting of measured
  reflectances using a Shifted Gamma micro-facet distribution", Computer
  Graphics Forum 31, 4, 2012: https://doi.org/10.1111/j.1467-8659.2012.03147.x

  Includes different variants:
  + sgd_base : unnormalized NDF without shadowing or masking
  + sgd : NDF with fitted normalization and and approximate shadowing and masking
  
*************************************************************************/

namespace bbm {
  namespace ndf {

    /********************************************************************/
    /*! \brief The unnormalized SGD microfacet distribution and
        without shadowing and masking

      \tparam CONF = bbm configuration
      \tparam NAME = ndf name (default = SGD_unnormalized)
      
      Implements: concepts::ndf
    *********************************************************************/
    template<typename CONF, string_literal NAME="SGD_base"> requires concepts::config<CONF>
      class sgd_base
    {
    public:
      BBM_IMPORT_CONFIG( CONF );
      static constexpr string_literal name = NAME;
      
      /******************************************************************/
      /*! \brief Evaluate the NDF

        \param halfway = vector to eval the NDF for
        \param mask = enable/disbale lanes [default = true]
        \returns the resulting evaluation of the NDF
      *******************************************************************/
      Spectrum eval(const Vec3d& halfway, Mask mask=true) const
      {
        // above surface?
        mask &= (vec::z(halfway) > 0);
        
        // Quick exit
        if(bbm::none(mask)) return 0;
        
        // Eval
        Spectrum temp = alpha + Spectrum(spherical::tanTheta2(halfway)) / alpha;
        Spectrum denum = bbm::pow(temp, p);
        Spectrum P22 = bbm::select(denum > Constants::Epsilon(), bbm::exp( -temp ) / denum, 0);

        // Done.
        return bbm::select(mask, P22 / (Constants::Pi() * bbm::pow(spherical::cosTheta(halfway), 4.0)), 0);
      }

      /******************************************************************/
      /*! \brief Sample the NDF
        
      \param view = view direction
      \param xi = 2D uniform random variables in [0..1] range
        \param Mask = enable/disbale lanes
        \returns A sampled microfacet normal.
      *******************************************************************/
      Vec3d sample(const Vec3d& view, const Vec2d& xi, Mask mask=true) const
      {
        // Use GGX for sampling with average alpha
        Value avg_alpha = bbm::hsum(alpha) / bbm::hsum(Spectrum(1));
        return ggx<Config>( avg_alpha ).sample(view, xi, mask);
      }

      /******************************************************************/
      /*! \brief PDF of sampling the NDF
        
        \param view = view direction
        \param m = sampled microfacet normal
        \param mask = enable/disable lanes [default = true]
        \returns the PDF of sampling 'm' using the sample method.
      *******************************************************************/
      Value pdf(const Vec3d& view, const Vec3d& m, Mask mask=true) const
      {
        // Use GGX for pdf with average alpha
        Value avg_alpha = bbm::hsum(alpha) / bbm::hsum(Spectrum(1));
        return ggx<Config>( avg_alpha ).pdf(view, m, mask);
      }
      
      
      /******************************************************************/
      /*! \brief Monodirectional shadowing and masking factor
        
        \param v = incident/exitant direction of transport
        \param m = microfacet normal.
        \param mask = enable/disable lanes [default = true]

        Bagher et al. did not present an analytical solution and instead
        relied on a fitted rational approximation of the shadowing and masking
        function.
      *******************************************************************/
      Value G1(const Vec3d& /*v*/, const Vec3d& /*m*/, Mask /*mask*/=true) const
      {
        return 1.0;
      }

      ///////////////////////////////
      //! @{ \name Class Attributes
      ///////////////////////////////
      specular_roughness<Spectrum> alpha;
      bsdf_parameter<Spectrum, bsdf_attr::SpecularParameter, 0.64> p;

      BBM_ATTRIBUTES( alpha, p );
      //! @}

      //! \brief Default Constructor
      BBM_DEFAULT_CONSTRUCTOR(sgd_base) {}
    };

    BBM_CHECK_CONCEPT( concepts::ndf, sgd_base<config> );

    
    /********************************************************************/
    /*! \brief The SGD microfacet distribution with fitted shadowing and masking

      \tparam CONF = bbm configuration
      \tparam NAME = ndf name (default = SGD)

      Implements: concepts::ndf
    *********************************************************************/
    template<typename CONF, string_literal NAME="SGD"> requires concepts::config<CONF>
      class sgd : public sgd_base<CONF>
    {
      using base = sgd_base<CONF>;
      BBM_BASETYPES(base);
    public:
      BBM_IMPORT_CONFIG( CONF );
      static constexpr string_literal name = NAME;

      //! @{ \name From base
      using base::sample;
      using base::pdf;
      //! @}
      
      /******************************************************************/
      /*! \brief Evaluate the NDF

        \param halfway = vector to eval the NDF for
        \param mask = enable/disbale lanes [default = true]
        \returns the resulting evaluation of the NDF
      *******************************************************************/
      Spectrum eval(const Vec3d& halfway, Mask mask=true) const
      {
        // above surface?
        mask &= (vec::z(halfway) > 0);
        
        // Quick exit
        if(bbm::none(mask)) return 0;

        // Eval
        return base::eval(halfway, mask) * K;
      }

      /******************************************************************/
      /*! \brief Monodirectional shadowing and masking term

        \param v = incident/outgoing vector
        \param m = microfacet normal
        \\param mask = enable/disable lanes
        \returns the monodirectional shadowing and masking attentuation factor
      *******************************************************************/
      Spectrum G1(const Vec3d& v, const Vec3d& m, Mask mask=true) const
      {
        // check (dot(v,m) / dot(v,n)) > 0.
        mask &= (vec::z(v) > 0) && (bbm::dot(v, m) > 0);
       
        // Quick exit
        if(bbm::none(mask)) return 0;

        // compute theta
        Value theta = spherical::theta(v);
        
        // Eval
        Spectrum g = bbm::select((theta > theta0), Scalar(1.0) + Lambda*(Scalar(1.0) - bbm::exp(c*bbm::pow(theta-theta0, k))), 1.0);

        // Done.
        return bbm::select(mask, g, 0);
      }

      ///////////////////////////////
      //! @{ \name Class Attributes
      ///////////////////////////////
      bsdf_parameter<Spectrum, bsdf_attr::Dependent, 7.5> K;
      bsdf_parameter<Spectrum, bsdf_attr::Dependent> Lambda;
      bsdf_parameter<Spectrum, bsdf_attr::Dependent> c;
      bsdf_parameter<Spectrum, bsdf_attr::Dependent> k;
      bsdf_parameter<Spectrum, bsdf_attr::Dependent, constants<Scalar>::Pi(0.5)> theta0;
      
      BBM_ATTRIBUTES( K, Lambda, c, theta0, k );
      //! @}

      //! \brief Default Constructor
      BBM_DEFAULT_CONSTRUCTOR(sgd) {}
    };

    BBM_CHECK_CONCEPT( concepts::ndf, sgd<config> );
    
  } // end ndf namespace
} // end bbm namespace

#endif /* _BBM_SDF_NDF_H_ */
