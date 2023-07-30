#ifndef _BBM_NDF_SAMPLER_H_
#define _BBM_NDF_SAMPLER_H_

#include "bbm/ndf.h"
#include "util/cdf.h"

/************************************************************************/
/*! \file sampler.h

  \brief Replace an NDF's sample and pdf method by a data-driven version.
  Monitors the NDF and update the internal state if required.

  Assumes an isotropic NDF!
  
************************************************************************/

namespace bbm {
  namespace ndf {

    /*******************************************************************/
    /*! \brief Replace an NDFs sample and pdf method with a data-driven
        numerical approximation.

        \tparam NDF = NDF for which to replace sample and pdf.

        \tparam samplesTheta = number of samples to take along the theta angle
                               of the halfway vector (detault = 90).

        \tparam samplesPhi = number of phi angles to *average* theta samples
        over (default = 1).

        \tparam NAME = name of the NDF (default is NDF::name + '_sampler').
    ********************************************************************/      
    template<typename NDF, size_t samplesTheta=90, size_t samplesPhi=1, string_literal NAME=NDF::name + string_literal("_sampler")> requires concepts::ndf<NDF>
      class sampler : public NDF
    {
      BBM_BASETYPES( NDF );
    public:
      BBM_IMPORT_CONFIG( NDF );
      static constexpr string_literal name = NAME;

      //! \brief forward copy eval and attenuation
      using NDF::NDF;
      using NDF::eval;
      using NDF::G1;

    /********************************************************************/
    /*! \brief Sample the NDF

      \param view = view direction (ignored)
      \param xi = 2D uniform random variables in [0..1] range
      \param Mask = enable/disbale lanes
      \returns A sampled microfacet normal.

      To avoid discontunuities, we sample according to a linear interpolation
      of the discrete PDF. Linear interpolation is achieved by distributing
      the samples, given a discrete bin, according to a linear-tent
      distribution centered at the mid-point of the bin, and extending to the
      middle of the neighboring bins. At the edges (0 and Pi/2), we reflect
      samples that exceed the hemispherical bounds.

     ********************************************************************/
    Vec3d sample(const Vec3d& /*view*/, const Vec2d& xi, Mask mask=true) const
    {
      // check if xi is within bounds
      mask &= (xi[0] >= 0) && (xi[0] <= 1) && (xi[1] >= 0) && (xi[1] <= 1);

      // quick bail out
      if(bbm::none(mask)) return 0.0;

      // reinit sampler?
      initialize();

      // sample cdf
      auto [index, residual] = bbm::pick<"index", "residual">( _cdf.sample(xi[0], bbm::cast<index_mask_t<Value>>(mask)) );

      // Linear interpolation with neighboring bins to avoid discontinuities in the pdf
      auto xi_r = bbm::abs(residual - 0.5);
      auto sample_offset = 1 - bbm::safe_sqrt(1 - 2*xi_r);  // [0...1]

      // sample halfway
      Vec2d h;
      spherical::theta(h) = bbm::pow( (index + 0.5 + bbm::sign(residual-0.5)*sample_offset) / samplesTheta, 2.0 ) * Constants::Pi(0.5);
      spherical::phi(h) = Constants::Pi(2) * xi[1];

      // reflect if below horizon (reflection at the zenith is automatic due
      // to the the symmetry around the Z axis).
      spherical::theta(h) = bbm::select(spherical::theta(h) > Constants::Pi(0.5), Constants::Pi() - spherical::theta(h), spherical::theta(h));
      
      // Done.
      return bbm::select(mask, spherical::convert(h), 0.0);
    }

    /********************************************************************/
    /*! \brief PDF of sampling the NDF

      \param view = view direction (ignored)
      \param m = sampled microfacet normal
      \param mask = enable/disable lanes [default = true]
      \returns the PDF of sampling 'm' using the sample method.
    *********************************************************************/
    Value pdf(const Vec3d& /*view*/, const Vec3d& m, Mask mask=true) const
    {
      // m above surface?
      mask &= (vec::z(m) > 0);

      // quick bail out
      if(bbm::none(mask)) return 0;

      // reinit sampler?
      initialize();

      // compute cdf index
      Value theta = spherical::theta(m);
      Value theta_index = bbm::sqrt(theta / Constants::Pi(0.5)) * samplesTheta - 0.5;

      // linear interpolate the cdf's pdf.
      Value w = theta_index - bbm::floor(theta_index);
      Size_t lidx = bbm::clamp( bbm::cast<Size_t>(bbm::floor(theta_index)), 0, samplesTheta-1); // handle reflection at horizon
      Size_t uidx = bbm::clamp( bbm::cast<Size_t>(bbm::ceil(theta_index)), 0, samplesTheta-1);  // handle reflection at zenith
      Value pdf = _cdf.pdf( lidx, bbm::cast<index_mask_t<Value>>(mask) ) * (1-w) + _cdf.pdf( uidx, bbm::cast<index_mask_t<Value>>(mask)) * w;
      
      // include Jacobian determinants
      Value jac = (bbm::sqrt(theta) * Constants::Pi2(0.25) / samplesTheta) * bbm::abs(bbm::sin(theta)) * (Constants::Pi(2.0));
      
      // Done.
      return bbm::select(mask && (jac > Constants::Epsilon()), pdf / jac, 0);
    }
    
    private:
      /******************************************************************/
      /*! \brief initialize the data structures

        Sample along the sqrt(theta) angle so that samples are more densely
        placed near the specular peak. Because a NDF is a decreasing function,
        we place the sample at the beginning of a sample 'bin'. We compensate
        the non-linear sampling by scaling the sample with a compensation
        factor. Because the weighting is increases, we sample at the end of
        the 'bin' to avoid missing important samples (i.e., max(f*g) <
        max(f)*max(g)).
        
       ******************************************************************/
      inline void initialize(void) const
      {
        // check if the CDF needs to be initialized or updated
        if(_cdf.size() != 0 && _monitor == bbm::reflection::attributes(*this)) return;

        // initialize _monitor
        _monitor = bbm::reflection::attributes(*this);

        // sample the NDF
        std::vector<Value> samples;

        Vec2d h;
        for(size_t thetaIdx=0; thetaIdx != samplesTheta; ++thetaIdx)
        {
          Value sample = 0;
          spherical::theta(h) = bbm::pow(Scalar(thetaIdx) / Scalar(samplesTheta), 2.0) * Constants::Pi(0.5);
            
          for(size_t phiIdx=0; phiIdx != samplesPhi; ++phiIdx)
          {
            spherical::phi(h) = Scalar(phiIdx) / Scalar(samplesPhi) * Constants::Pi(2);
            sample += static_cast<const NDF>(*this).eval( spherical::convert(h) );
          }

          // average over phi samples
          sample /= samplesPhi;

          // Weight to compensate for non-linear sampling
          Scalar theta1 = bbm::pow(Scalar(thetaIdx + 1) / Scalar(samplesTheta), 2.0) * Constants::Pi(0.5) ;
          sample *= bbm::sin(theta1) * bbm::sqrt(theta1);
          
          // store
          samples.push_back(sample);
        }
          
        // Construct the CDF
        _cdf = cdf(samples);
        
        // Done.
      }

      //////////////////////
      // Class Attributes
      //////////////////////
      mutable value_copy_tuple_t<anonymize_t<bbm::reflection::attributes_t<NDF>>> _monitor; //< copy of NDF parameters to check if _cdf needs to be reinit.
      mutable cdf<std::vector<Value>> _cdf;                                                 //< sampled cdf of the NDF
    };
      
  } // end ndf namespace
} // end bbm namespace

#endif /* _BBM_NDF_SAMPLER_H_ */
