#include <iostream>
#include <vector>
#include <random>

#include "bbm.h"
#include "bbm/bsdf_ptr.h"
#include "bbm/vec3dpair.h"
#include "core/spherical.h"
#include "util/option.h"
#include "util/multirange_for.h"
#include "util/gamma.h"

#ifdef BBM_PYTHON
  #include "python/bbm_python_interpreter.h"
#elif !defined(BBM_BSDF)
  // Change BBM_BSDF if a different BSDF is needed.
  #define BBM_BSDF lambertian<floatRGB>()
#endif /* BBM_PYTHON */


using namespace bbm;
BBM_IMPORT_CONFIG( floatRGB );


// random number generator
std::mt19937 rnd;

//! \brief Generate a random 2D point
Vec2d rndVec2d(void)
{
  std::uniform_real_distribution<float> U(0,1);
  return Vec2d(U(rnd), U(rnd));
}

//! \brief Generate point uniformly on sphere (pdf = 1/4pi)
BsdfSample sampleSphere(const Vec2d& xi)
{
  Vec2d coord;
  spherical::theta(coord) = bbm::safe_acos(1.0 - 2.0 * xi[0]);
  spherical::phi(coord) = xi[1] * Constants::Pi(2);

  return BsdfSample{ spherical::convert(coord), 1.0 / Constants::Pi(4), BsdfFlag::None };
}

//! \brief Generate point uniformly on hemisphere (pdf = 1/2pi)
BsdfSample sampleHemisphere(const Vec2d& xi)
{
  Vec2d coord;
  spherical::theta(coord) = bbm::safe_acos(xi[0]);
  spherical::phi(coord) = xi[1] * Constants::Pi(2);

  return BsdfSample{ spherical::convert(coord), 1.0 / Constants::Pi(2), BsdfFlag::None };
}


/***********************************************************************/
/*! \brief Compare reflectance with MC integral
************************************************************************/
void testReflectance(const BsdfPtr& bsdf, const option_map& opt)
{
  //////////////////
  // Parse Options
  //////////////////
  size_t samples = bbm::get<int_option>(opt, "samples", 100000);
  size_t numtheta = bbm::get<int_option>(opt, "theta", 1);
  bool importance = bbm::get<bool_option>(opt, "importanceSampling");

  auto invalid = validate_option_keywords(opt, "bsdfmodel", "test", "samples", "theta", "importanceSampling");
  if(invalid != "")
  {
    std::cerr << "ERROR: invalid keyword '" << invalid << "'." << std::endl;
    return;
  }
  
  std::cout << "Reflectance test with " << numtheta << " directions and " << samples << " samples." << std::endl;
  
  // for each theta
  Vec2d out_sp(0);
  for(size_t  theta_idx = 0; theta_idx < numtheta; ++theta_idx)
  {
    // get out vector
    spherical::theta(out_sp) = theta_idx * Constants::Pi(0.5) / numtheta;
    Vec3d out = spherical::convert(out_sp);

    // integrate over in projected hemisphere (use either cosine sampling (importance == false) or importance sampling according to the BSDF (importance == true)
    Spectrum estimate(0);
    for(size_t s=0; s < samples; ++s)
    {
      // generate random dir
      auto sample = (importance) ? bsdf.sample(out, rndVec2d()) : sampleSphere(rndVec2d());
      
      // accumulate
      if(sample.pdf > Constants::Epsilon())
        estimate += bsdf.eval(sample.direction, out) * vec::z(sample.direction) / sample.pdf;
    }
    
    // normalize
    estimate /= samples;
    
    // compare results:
    std::cout << " out = " << out << " => Estimate: " << estimate << " vs. " << bsdf->reflectance(out) << std::endl;
  }

  // Done.
}

/***********************************************************************/
/*! \brief Check reciprocity
************************************************************************/
void testReciprocity(const BsdfPtr& bsdf, const option_map& opt)
{
  ///////////////////
  // Parse Options
  ///////////////////
  size_t samples = get<int_option>(opt, "samples", 1000000);

  auto invalid = validate_option_keywords(opt, "bsdfmodel", "test", "samples");
  if(invalid != "")
  {
    std::cerr << "ERROR: invalid keyword '" << invalid << "'." << std::endl;
    return;
  }

  std::cout << "Reciprocity test with " << samples << " samples." << std::endl;

  // compute average and max difference over all samples
  Spectrum sum_r = 0, max_r = 0;
  Spectrum sum_i = 0, max_i = 0;
  Vec3dPair maxpair_i = {0,0}, maxpair_r = {0,0};
  
  for(size_t s=0; s < samples; ++s)
  {
    // get random direction pair (uniform)
    auto sample1 = sampleSphere( rndVec2d() );
    auto sample2 = sampleSphere( rndVec2d() );
    Vec3dPair dir = {sample1.direction, sample2.direction};
    
    // compute difference
    Spectrum diff_r = bbm::abs( bsdf.eval(dir.in, dir.out, bsdf_flag::All, unit_t::Radiance) - bsdf.eval(dir.out, dir.in, bsdf_flag::All, unit_t::Radiance) );
    Spectrum diff_i = bbm::abs( bsdf.eval(dir.in, dir.out, bsdf_flag::All, unit_t::Importance) - bsdf.eval(dir.out, dir.in, bsdf_flag::All, unit_t::Importance) );

    // accumulate
    sum_r += diff_r;
    sum_i += diff_i;

    // store max difference
    if( bbm::hsum(diff_r) > bbm::hsum(max_r) ) { maxpair_r = dir; max_r = diff_r; }
    if( bbm::hsum(diff_i) > bbm::hsum(max_i) ) { maxpair_i = dir; max_i = diff_i; }
  }

  // normalize
  sum_r /= samples;
  sum_i /= samples;

  // output results:
  std::cout << "Radiance   average = " << sum_r << ", max = " << max_r << " at " << maxpair_r << std::endl;
  std::cout << "Importance average = " << sum_i << ", max = " << max_i << " at " << maxpair_i << std::endl; 
  
  // Done.
}

/***********************************************************************/
/*! \brief Check adjoint
************************************************************************/
void testAdjoint(const BsdfPtr& bsdf, const option_map& opt)
{
  ///////////////////
  // Parse Options
  ///////////////////
  size_t samples = get<int_option>(opt, "samples", 100000);

  auto invalid = validate_option_keywords(opt, "bsdfmodel", "test", "samples");
  if(invalid != "")
  {
    std::cerr << "ERROR: invalid keyword '" << invalid << "'." << std::endl;
    return;
  }

  std::cout << "Adjoint test with " << samples << " samples." << std::endl;

  // compute average and max difference over all samples
  Spectrum sum_a = 0, max_a = 0;
  Vec3dPair maxpair_a = {0,0};
  
  for(size_t s=0; s < samples; ++s)
  {
    // get random direction pair (uniform)
    auto sample1 = sampleSphere( rndVec2d() );
    auto sample2 = sampleSphere( rndVec2d() );
    Vec3dPair dir = {sample1.direction, sample2.direction};
    
    // compute difference
    Spectrum diff_a = bbm::abs( bsdf.eval(dir.in, dir.out, bsdf_flag::All, unit_t::Radiance) - bsdf.eval(dir.out, dir.in, bsdf_flag::All, unit_t::Importance) );

    // accumulate
    sum_a += diff_a;

    // store max difference
    if( bbm::hsum(diff_a) > bbm::hsum(max_a) ) { maxpair_a = dir; max_a = diff_a; }
  }

  // normalize
  sum_a /= samples;

  // output results:
  std::cout << "Adjoint difference average = " << sum_a << ", max = " << max_a << " at " << maxpair_a << std::endl;
  
  // Done.
}

/***********************************************************************/
/*! \brief Check PDF properties (non-negative & sample.pdf == pdf)
************************************************************************/
void testPdf(const BsdfPtr& bsdf, const option_map& opt)
{
  ///////////////////
  // Parse Options
  ///////////////////
  size_t samples = get<int_option>(opt, "samples", 100000);
  size_t maxError = get<int_option>(opt, "maxError", 10);
  bool checkBelowHorizon = get<bool_option>(opt, "checkBelowHorizon");
  bool samplesphere = get<bool_option>(opt, "sampleSphere");
  
  auto invalid = validate_option_keywords(opt, "bsdfmodel", "test", "samples", "maxError", "checkBelowHorizon", "sampleSphere");
  if(invalid != "")
  {
    std::cerr << "ERROR: invalid keyword '" << invalid << "'." << std::endl;
    return;
  }


  std::cout << "Tesing PDF properties test with " << samples << " samples." << std::endl;
  
  // warn if negative PDF, and check if sample.pdf == pdf
  size_t count_negative_r = 0, count_negative_i = 0;
  size_t count_zr = 0, count_zi = 0;
  Scalar mismatch_r = 0, mismatch_i = 0;

  for(size_t s = 0 ; s < samples && count_negative_r < maxError && count_negative_i < maxError && count_zr < maxError && count_zi < maxError; ++s)
  {
    // uniform random dir
    auto sample = (samplesphere) ? sampleSphere(rndVec2d()) : sampleHemisphere( rndVec2d() );

    // sample bsdf
    auto sample_r = bsdf.sample(sample.direction, rndVec2d(), bsdf_flag::All, unit_t::Radiance);
    auto sample_i = bsdf.sample(sample.direction, rndVec2d(), bsdf_flag::All, unit_t::Importance);

    // below horizong?
    if(checkBelowHorizon && vec::z(sample_r.direction) < 0) { count_zr++; std::cout << " Sampled direction " << sample_r.direction << " below horizon for " << sample.direction << std::endl; }
    if(checkBelowHorizon && vec::z(sample_i.direction) < 0) { count_zi++; std::cout << " Sampled direction " << sample_i.direction << " below horizon for " << sample.direction << std::endl; }
    
    // eval pdf
    auto pr = bsdf.pdf(sample_r.direction, sample.direction, bsdf_flag::All, unit_t::Radiance);
    auto pi = bsdf.pdf(sample_i.direction, sample.direction, bsdf_flag::All, unit_t::Importance);

    // check sign
    if(pr < 0) { count_negative_r++; std::cout << " Negative PDF (" << pr << ") for (" << sample_r.direction << ", " << sample.direction << ")" << std::endl; }
    if(pi < 0) { count_negative_i++; std::cout << " Negative PDF (" << pi << ") for (" << sample_i.direction << ", " << sample.direction << ")" << std::endl; }
    
    // check mismatch
    mismatch_r += bbm::abs(sample_r.pdf - pr);
    mismatch_i += bbm::abs(sample_i.pdf - pi);
  }

  // normalize
  mismatch_r /= samples;
  mismatch_i /= samples;
  
  // output results
  std::cout << "PDF has " << count_negative_r << "/" << count_negative_i << " negative PDF values, ";
  if(checkBelowHorizon) std::cout << count_zr << "/" << count_zi << " sampled directions below the horizon, ";
  std::cout << "and " << mismatch_r << "/" << mismatch_i << " average difference between the PDF from the sample method and the corresponding PDF from the pdf-method." << std::endl;
  
  // Done.
}

/***********************************************************************/
/*! \brief Check PDF Integral == 1
************************************************************************/
void testPdfInt(const BsdfPtr& bsdf, const option_map& opt)
{
  ///////////////////
  // Parse Options
  ///////////////////
  size_t samples = get<int_option>(opt, "samples", 1000000);
  size_t trials = get<int_option>(opt, "trials", 10);
  bool samplesphere = get<bool_option>(opt, "sampleSphere");

  auto invalid = validate_option_keywords(opt, "bsdfmodel", "test", "samples", "trials", "sampleSphere");
  if(invalid != "")
  {
    std::cerr << "ERROR: invalid keyword '" << invalid << "'." << std::endl;
    return;
  }
    
  std::cout << "Tesing PDF Integral with " << samples << " samples, for " << trials << " random directions sampled over the " << ((samplesphere) ? "sphere" : "hemisphere") << std::endl;

  // generate 'trials' random directions to check the PDF integral for
  for(size_t t=0; t < trials; ++t)
  {
    Scalar pdf_r = 0, pdf_i = 0;
    auto sample_t = (samplesphere) ? sampleSphere(rndVec2d()) : sampleHemisphere(rndVec2d());

    for(size_t s=0; s < samples; ++s)
    {
      auto sample_s = sampleSphere(rndVec2d());

      if(sample_s.pdf > Constants::Epsilon())
      {
        pdf_r += bsdf.pdf(sample_s.direction, sample_t.direction, bsdf_flag::All, unit_t::Radiance) / sample_s.pdf;
        pdf_i += bsdf.pdf(sample_s.direction, sample_t.direction, bsdf_flag::All, unit_t::Importance) / sample_s.pdf;
      }
    }

    // normalize
    pdf_r /= samples;
    pdf_i /= samples;

    // output
    std::cout << " Integral = " << pdf_r << "/" << pdf_i << " (radiance/importance) for " << sample_t.direction << std::endl;
  }

  // Done.
}

/***********************************************************************/
/*! \brief Check whether sampling matches pdf (White Furnace Test - ChiSquare test)
************************************************************************/
void testSample(const BsdfPtr& bsdf, const option_map& opt)
{
  ///////////////////
  // Parse Options
  ///////////////////
  size_t pdfSamples = get<int_option>(opt, "pdfSamples", 4096);
  size_t samples = get<int_option>(opt, "samples", 100000);
  size_t theta = get<int_option>(opt, "theta", 10);
  size_t phi = get<int_option>(opt, "phi", 20);
  size_t trials = get<int_option>(opt, "trials", 10);
  bool samplesphere = get<bool_option>(opt, "sampleSphere");
  bool includeZeroPdfSamples = get<bool_option>(opt, "includeZeroPdfSamples");
  
  auto invalid = validate_option_keywords(opt, "bsdfmodel", "test", "pdfSamples", "samples", "theta", "phi", "trials", "sampleSphere", "includeZeroPdfSamples");
  if(invalid != "")
  {
    std::cerr << "ERROR: invalid keyword '" << invalid << "'." << std::endl;
    return;
  }

  std::cout << "Testing if sample and pdf match: " << pdfSamples << " PDF samples per bin, and " << samples << " direction samples, with (" << phi << " x " << theta << ") bins over " << trials << " trials";
  if(includeZeroPdfSamples) std::cout << ", including zero pdf samples";
  std::cout << "." << std::endl;
  
  for(size_t tr=0; tr < trials; ++tr)
  {
    auto sample_t = (samplesphere) ? sampleSphere(rndVec2d()) : sampleHemisphere(rndVec2d());    

    // compute pdf per bin
    std::vector<Scalar> pdf(theta * phi, 0);
    std::vector<Scalar> count(theta * phi, 0);
  
    // integrate pdf for all bins
    Vec2d sph_coord;
    size_t idx=0;
    for(size_t t=0; t < theta; ++t)
      for(size_t p=0; p < phi; ++p, ++idx)
      {
        for(size_t s=0; s < pdfSamples; ++s)
        {
          // sample
          auto rnd = rndVec2d();
          spherical::phi(sph_coord) = Constants::Pi(2) * (p + rnd[0]) / phi;
          spherical::theta(sph_coord) = Constants::Pi() * (t + rnd[1]) / theta;
          Vec3d dir = spherical::convert(sph_coord);
          Value p = Constants::Pi2(2) * bbm::abs(spherical::sinTheta(sph_coord)) / (phi * theta);
          
          // store
          pdf[idx] += bsdf.pdf(dir, sample_t.direction ) * p;
        }
        pdf[idx] /= pdfSamples;
      }

    // count samples per bin
    for(size_t s=0; s < samples; ++s)
    {
      auto sample = bsdf.sample(sample_t.direction, rndVec2d());

      if(includeZeroPdfSamples || sample.pdf > Constants::Epsilon())
      {
        sph_coord = spherical::convert(sample.direction);
        size_t t = bbm::min( spherical::theta(sph_coord) / Constants::Pi() * theta, theta-1);
        size_t p = bbm::min( spherical::phi(sph_coord) / Constants::Pi(2) * phi, phi-1);
        size_t idx = t * phi + p;

        // store
        count[idx]++;
      }
    }

    // compute chi-square
    Scalar chi2 = 0, df = -1; idx=0;
    for(size_t t=0; t < theta; ++t)
      for(size_t p=0; p < phi; ++p, idx++)
      {
        Scalar m = pdf[idx] * samples;
        
        if(m > Constants::Epsilon() && count[idx] > 5)
        {
          //std::cerr << pdf[idx] << ": " << count[idx] << " vs " << m << std::endl;
          chi2 += pow( count[idx] - m, 2) / m;
          df++;
        }
      }

    // output
    std::cout << " Chi2 for " << sample_t.direction << " = " << chi2 << " (with " << df << " degrees of freedom)." << std::endl;

    // compute P value
    if(df > 1)
      {
      float P = bbm::gamma_q((df-1) / 2, chi2 / 2);
      std::cout << "  P = " << P << " (reject if lower than confidence)." << std::endl;
    }
    else std::cout << " No degrees of freedom; need at least 1 to compute P." << std::endl;
  }
  
  // Done.
}
  


/***********************************************************************/
/*! \brief main function

  Usage: checkBsdf <test name> [param]

  Available tests:
  + reflectance [samples=100,000] [num_theta_out=1] [use importance sampling=0]
  + reciprocity [samples=100,000]
  + adjoint [samples=100,000]
  + pdf [samples=100,000] [show=10] [checkBelowHorizon=false] [sample sphere=false]
  + pdfInt [samples = 100,000] [tries=10] [sample sphere=false]
  + sample [phi_samples=4069] [chi_samples=5] [binsTheta=10] [binsPhi=20] [tries=10] [sample sphere=false]
***********************************************************************/
int main(int argc, char** argv)
{
  /////////////////
  // Parse Input //
  /////////////////
  if(argc == 1)
  {
    std::cout << "Usage: " << argv[0] << " [bsdfmodel=<python string>] [test=<test name> [test options]" << std::endl;
    std::cout << "  + test=reflectance [samples=100000] [theta=1] [importanceSampling]: compare the approximated reflectance method with a MC integration of the BSDF." << std::endl;
    std::cout << "  + test=reciprocity [samples=100000]: checks if the BSDF is symmetric for 'samples' random dirctions." << std::endl;
    std::cout << "  + test=adjoint [samples=100000]: checks if the adjoint BSDF is equal to the BSDF with in/out swapped." << std::endl;
    std::cout << "  + test=pdf [samples=100000] [maxError=10] [checkBelowHorizon] [sampleSphere]: checks if the PDF >= 0, and the PDF returned by the sampling method matches the pdf from the pdf-method. Abort if the number of fails exceeds 'maxError'" << std::endl;
    std::cout << "  + test=pdfInt [samples=100000] [trials=10] [sampleSphere]: checks the integral (MC with 'samples' samples) of the PDF for 'trials' different directions." << std::endl;
    std::cout << "  + test=sample [pdfSamples=4069] [samples=100000] [theta=10] [phi=20] [trials=10] [sampleSphere] [includeZeroPdfSamples]: perform Chi2 test on the sample vs the pdf method.  The domain is subdivided in [theta x phi] bins, and for each bin we integrate the PDF using MC.  A higher sampling rate might be needed for sharp BSDFs." << std::endl;
#ifndef BBM_PYTHON
    std::cout << "BBM compiled without Python support. <bsdfmodel> is ignored and " << BBM_STRINGIFY(BBM_BSDF) << " is used instead." << std::endl;
#endif /* BBM_PYTHON */
    return -1;
  }

  auto opt = parse_options(argc, argv);
  auto bsdfmodel = get<string_option>(opt, "bsdfmodel");
  auto testname = get<string_option>(opt, "test");

  if(testname == "")
  {
    std::cout << "ERROR: no test specified." << std::endl;
    return -1;
  }

  
  /////////////////
  // create bsdf
  /////////////////
  #ifdef BBM_PYTHON
    if(bsdfmodel == "")
    {
      std::cout << "ERROR: bsdfmodel not specified." << std::endl;
      return -1;
    }
    BsdfPtr bsdf = python::capture<BsdfPtr>(bsdfmodel);
    std::cout << "Using BSDF: " << bsdf << std::endl;
  #else
    if(bsdfmodel != "")
      std::cout << "Ignoring user specified BSDF (" << bsdfmodel << "), and using BBM_BSDF=" << BBM_STRINGIFY(BBM_BSDF) << " instead." << std::endl;
    BsdfPtr bsdf = make_bsdf_ptr(BBM_BSDF);
  #endif /* BBM_PYTHON */
  
  // run test
  if(testname == "reflectance") testReflectance(bsdf, opt);
  else if(testname == "reciprocity") testReciprocity(bsdf, opt);
  else if(testname == "adjoint") testAdjoint(bsdf, opt);
  else if(testname == "pdf") testPdf(bsdf, opt);
  else if(testname == "pdfInt") testPdfInt(bsdf, opt);
  else if(testname == "sample") testSample(bsdf, opt);
  else std::cout << "Unrecognized test: '" << testname << "'" << std::endl;
  
  // Done.
  return 0;
}
