/************************************************************************/
/*! \file convolution.cpp

  \brief Validate various component in the precomputation of Eq 61-64 from "A
  two-scale microfacet reflectance model combining reflection and
  diffraction", Holzschuch and Pacanowski [2017]:
  https://doi.org/10.1145/3072959.3073621)

*************************************************************************/

#include <iostream>
#include <random>
#include <vector>

#include "bbm.h"
#include "optimizer/compass.h"
using namespace bbm;

BBM_IMPORT_CONFIG(floatRGB);

// random number generator
std::mt19937 rnd;

// parameter distributions
std::uniform_real_distribution<Value> urange(0, 1.0);          // can be much larger than 1.
std::uniform_real_distribution<Value> crange(1.1, 3.02);
std::uniform_real_distribution<Value> betarange(Constants::Epsilon(), 0.5);
std::uniform_real_distribution<Value> prange(0.05, 5);
std::uniform_real_distribution<Value> brange(0.01, 100.0);
std::uniform_real_distribution<Value> lambdarange(0.380, 0.780);

// Config (HP=true,false,true,false,false)
constexpr bool normalize_epd = true;     // normalize EPD computations
constexpr bool normalize_abc = false;    // normalize ABC computations
constexpr bool epd_scale = true;         // scale fit by EPD normalization
constexpr bool abc_scale = false;        // scale fit by ABC normalization
constexpr bool cos_scale = false;        // experimental

/************************************************************************/
/*! \brief Eval Zonal Harmonics

  \param l = degree
  \param c = cosine theta
************************************************************************/
inline Value ZH(size_t l, const Value& c)
{
  auto normalization = bbm::sqrt((2.0*l + 1.0) * Constants::InvPi(0.25));
  auto legendre = std::legendre(l, c);
  return normalization * legendre;
}


/***********************************************************************/
/*! \brief Eq (16); assumes b=1
************************************************************************/
template<bool NORMALIZE=normalize_abc>
  inline Value S_HS(Value f2, Value c, Value b=1)
{
  auto b2 = b*b;
  auto eval = bbm::pow(1.0 + b2*f2, -0.5*(c+1.0));
  if constexpr (NORMALIZE) eval *= (c-1.0) * Constants::InvPi(0.5) * b2;
  return eval; 
}

/***********************************************************************/
/*! \brief Eq (32.5); assumes b & lambda=1
  \param cosTheta = cos theta half
  \param cos = cos(theta_d)
  \param c = c parameter
  \param b = b parameter
  \param lambda = wavelength
************************************************************************/
template<bool NORMALIZE=normalize_abc>
  inline Value S(Value cosTheta, Value cosd, Value c, Value b=1, Value lambda=1)
{
  auto u = 1.0/lambda*cosd;
  auto c2 = cosTheta*cosTheta;
  auto s2 = 1.0 - c2;
  auto f2 = 4.0 * u*u * s2;      // f = 2/lambda cosd sinh  (Eq. 9)
  return S_HS<NORMALIZE>(f2, c, b) * c2;
}

/************************************************************************/
/*! \brief Eq. 39; assumes b and lambda are included in 'u' for look up of s and g
  \param cosTheta = cos theta half
  \param param = fitted parameters s(u,c,beta,p) and g(u,c,beta,p)
  \param c = c parameter
*************************************************************************/
template<bool NORMALIZE=normalize_abc, bool CosScale=cos_scale>
inline Value evalFit(Value cosTheta, const Vec2d& param, Value c, Value b=1, Value lambda=1)
{
  auto c2 = cosTheta*cosTheta;
  auto s2 = 1.0 - c2;
  auto f2 = 4.0 * param[1]*param[1] * s2 / (b*b); // * (lambda*lambda);
  auto eval = param[0] * S_HS<NORMALIZE>(f2, c, b);
  if constexpr (CosScale) eval *= c2;
  return eval;
}


/************************************************************************/
/*! \brief Eval the EPD function

  \param cosTheta = cos(theta_h) at which to evaluate the EPD
  \param beta = beta parameter of the model (i.e., roughess)
  \param p = p parameter (i.e., kurtosis)
*************************************************************************/
template<bool NORMALIZE=normalize_epd>
  inline Value EPD(Value cosTheta, Value beta, Value p)
{
  auto c2 = cosTheta * cosTheta;
  auto s2 = 1.0 - c2;
  auto eval = bbm::exp(-bbm::pow(s2 / (c2 * beta * beta), p)) / (c2*c2);
  if constexpr (NORMALIZE)
    eval *= p * Constants::InvPi() * bbm::rcp(bbm::tgamma(1.0 / p) * beta * beta);
  return eval;
}

/************************************************************************/
/*! \brief Brute force computation of the zonal harmonics coefficient of
    degree l of the exponential power distribution NDF.

    \param l = ZH degree
    \param beta = width of EPD lobe
    \param p = kurtosis of EPD lobe
    \param samples = number of MC samples
*************************************************************************/
template<bool NORMALIZE=normalize_epd>
  inline Value bruteforce_ZH_EPD(size_t l, const Value& beta, const Value& p, size_t samples=1000000)
{
  std::uniform_real_distribution<Value> U(0,Constants::Pi(0.5));
  
  Value integral = 0;  
  for(size_t s=0; s < samples; ++s)
  {
    Value theta = U(rnd);
    Value c = bbm::cos(theta);
    Value c2 = c*c;
    Value s2 = 1.0 - c2;
    
    integral += EPD<NORMALIZE>(c, beta, p) * ZH(l, c) * bbm::safe_sqrt(s2);
  }

  integral *= Constants::Pi2() / Value(samples);

  // Done.
  return integral;
}


/************************************************************************/
/*! \brief Uses the midpoint rule to compute the integral between a zonal
    harmonics of degree l and the exponential power distribution NDF with
    parameters beta and p.

    \param l = ZH degree
    \param beta = width of EPD lobe
    \param p = kurtosis of EPD lobe

*************************************************************************/
template<bool NORMALIZE=normalize_epd>
  inline Value bbm_ZH_EPD(size_t l, const Value& beta, const Value& p)
{
  Value integral=0;
  Value delta = 1.0 / 600000.0;
  for(Value cosTheta=1; cosTheta > 0; cosTheta -= delta)
    integral += EPD<NORMALIZE>(cosTheta,beta,p) * ZH(l,cosTheta) * delta;

  // Done.
  return integral * Constants::Pi(2.0);
}

/************************************************************************/
/*! \brief brute force computation of the zonal harmonics coefficients of
    degree l of the S function (S_HS(2cos(theta_d)sin(theta_h)) * cos^2(theta_h)) 

    \param l = ZH degree
    \param cosd = cos(theta_d)
    \param c = c parameter of the ABC model
    \param b = b parameter
    \param lambda = wavelength
    \param samples = number of MC samples

    Assumes lambda and b are 1. Does not include sigma_s^2
*************************************************************************/
template<bool NORMALIZE=normalize_abc>
  inline Value bruteforce_ZH_ABC(size_t l, const Value& cosd, const Value& c, const Value& b=1.0, const Value& lambda=1, size_t samples=1000000)
{
  std::uniform_real_distribution<Value> U(0,Constants::Pi(0.5));
  
  Value integral = 0;
  for(size_t s=0; s < samples; ++s)
  {
    Value theta_h = U(rnd);
    Value cosTheta = bbm::cos(theta_h);
    Value c2 = cosTheta*cosTheta;
    Value s2 = 1.0 - c2;

    integral += S<NORMALIZE>(cosTheta,cosd,c,b,lambda) * ZH(l, cosTheta) * bbm::safe_sqrt(s2);
  }

  integral *= Constants::Pi2() / Value(samples);

  // Done.
  return integral;
}


/************************************************************************/
/*! \brief Uses the midpoint rule to compute the integral between a zonal
    harmonics of degree l and the K-correlation model (ABC) with parameters b
    and c.

    \param l = ZH degree
    \param cosd = cos(theta_d)
    \param c = c parameter of the ABC model
    \param b = b parameter
    \param lambda = wavelength
    
*************************************************************************/
template<bool NORMALIZE=normalize_abc>
  inline Value bbm_ZH_ABC(size_t l, const Value& cosd, const Value& c, const Value& b=1, const Value& lambda=1)
{
  Value integral=0;
  Value delta = 1.0 / 600000.0;
  for(Value cosTheta=1; cosTheta > 0; cosTheta -= delta)
  {
    integral += S<NORMALIZE>(cosTheta, cosd, c, b, lambda) * ZH(l,cosTheta) * delta;
  }

  integral *= Constants::Pi(2.0);
  
  // Done.
  return integral;
}


/************************************************************************/
/*! \brief Evaluate a function expressed as ZH coefficients
*************************************************************************/
inline Value evalConv(const std::vector<Value>& coef, Value cosTheta)
{
  Value result=0;
  for(size_t l=0; l < coef.size(); ++l)
    result += coef[l] * ZH(l, cosTheta);
  return result;
}

/************************************************************************/
/*! \brief loss function for fitting ZH to K-correlation (meets concepts::lossfunction)
  Use scale to mimic the loss of HP reference code (== normalization of EPD)
*************************************************************************/
template<bool NORMALIZE=normalize_abc, bool CosScale=cos_scale>
  struct lossfunc
{
  BBM_IMPORT_CONFIG( floatRGB );

  lossfunc(const Vec2d& param, const std::vector<Value>& reference, Value c, Value scale) : _param(param), _reference(reference), _c(c), _scale(scale) {}
  void update(void) {}

  //! \brief Eval the loss
  Value operator()(Mask=true) const
  {
    Value err = 0;
    for(size_t s=0; s < _reference.size(); ++s)
    {
      auto sinTheta = 1.0 - Value(s) / Value(_reference.size());
      auto cosTheta = sqrt(1.0 - sinTheta*sinTheta);
      err += bbm::pow( evalFit<NORMALIZE,CosScale>(cosTheta, _param, _c)*_scale - _reference[s], 2.0 );
    }
    return err;
  }
    
private:
  Value _scale;
  Value _c;
  const Vec2d& _param;
  const std::vector<Value>& _reference;
};


/************************************************************************/
/*! \brief Compare the ZH coefficients of brute force MC computed versus
   midpoint.
*************************************************************************/
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc>
  void test_BF_vs_Midpoint_degree(size_t l, Value beta, Value p, Value u, Value c) 
{
  auto d = bruteforce_ZH_EPD<NORMALIZE_EPD>(l, beta, p) ;
  auto bd = bbm_ZH_EPD<NORMALIZE_EPD>(l, beta, p);

  auto s = bruteforce_ZH_ABC<NORMALIZE_ABC>(l, u, c) ;
  auto bs = bbm_ZH_ABC<NORMALIZE_ABC>(l, u, c);
    
  std::cerr << l << ": EPD(" << beta << ", " << p << ") = " << d << " vs. " << bd << " and ABC(" << u << ", " << c << ") = " << s << " vs. " << bs << std::endl;
}

//! \brief Bf vs midpoint for a number of degrees
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc>
  void test_BF_vs_Midpoint(size_t maxDegree, Value beta, Value p, Value u, Value c) 
{
  for(size_t l=0; l < maxDegree; ++l)
    test_BF_vs_Midpoint_degree<NORMALIZE_EPD, NORMALIZE_ABC>(l, beta, p, u, c);
}

//! \brief Bf vs midpoint for a series of parameters and degrees
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc>
  void test_BF_vs_Midpoint(size_t maxDegree) 
{
  for(size_t l=0; l < maxDegree; ++l)
  {
    Value beta = betarange(rnd);
    Value p = prange(rnd);
    Value u = urange(rnd);
    Value c = crange(rnd);
    test_BF_vs_Midpoint_degree<NORMALIZE_EPD, NORMALIZE_ABC>(l, beta, p, u, c);
  }
}

/************************************************************************/
/*! \brief Compare direct evalutaion versus ZH evaluation
*************************************************************************/
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc>
  void test_Eval_vs_ZH(Value beta, Value p, Value u, Value c, size_t maxDegree, size_t maxEval)
{
  std::cerr << "EPD(" << beta << ", " << p << "): ";
  std::vector<Value> coef_epd;
  for(size_t l=0; l < maxDegree; ++l)
    coef_epd.push_back( bbm_ZH_EPD<NORMALIZE_EPD>(l, beta, p) );
      
  for(size_t d=0; d < maxEval; ++d)
  {
    Value cosTheta = 1.0 - Value(d) / Value(maxEval);
    std::cerr << "(" << EPD<NORMALIZE_EPD>(cosTheta, beta, p) << " vs " << evalConv(coef_epd, cosTheta) << "), ";
  }
  std::cerr << std::endl;
    
  std::cerr << "S(" << u << ", " << c << "): ";
  std::vector<Value> coef_abc;
  for(size_t l=0; l < maxDegree; ++l)
    coef_abc.push_back( bbm_ZH_ABC<NORMALIZE_ABC>(l, u, c) );
  
  for(size_t d=0; d < maxEval; ++d)
  {
    Value cosTheta = 1.0 - Value(d) / Value(maxEval);
    std::cerr << "(" << S<NORMALIZE_ABC>(cosTheta, u, c) << " vs " << evalConv(coef_abc, cosTheta) << "), ";
  }
  std::cerr << std::endl << std::endl;
}

//! \brief Compare a series of parameters
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc>
  void test_Eval_vs_ZH(size_t maxTests, size_t maxDegree, size_t maxEval)
{
  for(size_t t=0; t < maxTests; ++t)
  {
    Value beta = betarange(rnd);
    Value p = prange(rnd);
    Value u = urange(rnd);
    Value c = crange(rnd);
    test_Eval_vs_ZH<NORMALIZE_EPD, NORMALIZE_ABC>(beta, p, u, c, maxDegree, maxEval);
  }
}

/************************************************************************/
/*! \brief Compare ZH convolution evaluation versus fitted evaluation
*************************************************************************/
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc, bool EPD_SCALE=epd_scale, bool ABC_SCALE=abc_scale, bool CosScale=cos_scale>
  void test_ZH_vs_Fit(Value beta, Value p, Value u, Value c, size_t maxDegree, size_t maxSamples, size_t maxItr, size_t maxEval)
{
  Value scale = 1.0;
  if constexpr (EPD_SCALE) scale *= p * Constants::InvPi() * bbm::rcp(bbm::tgamma(1.0 / p) * beta * beta);
  if constexpr (ABC_SCALE) scale *= (c-1.0) * Constants::InvPi(0.5);

  const Vec2d init(2.0 / scale, 0.5);
  
  // convolve EPD and ABC in ZH
  std::vector<Value> conv;
  for(size_t l=0; l < maxDegree; ++l)
    conv.push_back( bbm_ZH_EPD<NORMALIZE_EPD>(l, beta, p) * bbm_ZH_ABC<NORMALIZE_ABC>(l, u, c) * bbm::sqrt(Constants::Pi(4.0) / (2.0*l + 1.0)));

  // sample
  std::vector<Value> samples;
  for(size_t s=0; s < maxSamples; ++s)
  {
    Value sinTheta = 1.0 - Value(s) / Value(maxSamples);
    Value cosTheta = bbm::sqrt(1.0 - sinTheta*sinTheta);
    samples.push_back( evalConv(conv, cosTheta) );
  }

  // fit
  Vec2d param(init);
  lossfunc<NORMALIZE_ABC, CosScale> loss(param, samples, c, scale);
  compass opt(loss, param, Vec2d(0.0,0.0), Vec2d(10.0, 50.0), Constants::Epsilon(), 1.0, 0.5, 2.0);

  size_t i=0;
  for(i=0; i < maxItr && !opt.is_converged(); ++i)
  {
    auto err = opt.step();
    //std::cerr << i << ": " << param[0] << ", " << param[1] << ", " << err << std::endl;
  }
  
  // print warning if not converged
  if(!opt.is_converged())
    std::cout << " WARNING: optimization not converged (error = " << loss() << ")" << std::endl;

  // compare
  std::cerr << "Fit(u=" << u << ", c=" << c << ", p=" << p << ", beta=" << beta << ") - loss=" << loss() << ", param=" << param << ": ";
  for(size_t d=0; d != maxEval; ++d)
  {
    Value cosTheta = 1.0 - Value(d) / Value(maxEval);
    auto ec = evalConv(conv, cosTheta);
    auto ef = evalFit<NORMALIZE_ABC,CosScale>(cosTheta, param, c)*scale;
    std::cerr << "(" << ec << " vs " << ef << "), ";
  }
  std::cerr << std::endl;
}

//! \brief do maxTests ZH vs Fit
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc, bool EPD_SCALE=epd_scale, bool ABC_SCALE=abc_scale>
  void test_ZH_vs_Fit(size_t maxTests, size_t maxDegree, size_t maxSamples, size_t maxItr, size_t maxEval)
{
  for(size_t t=0; t < maxTests; ++t)
  {
    Value beta = betarange(rnd);
    Value p = prange(rnd);
    Value u = urange(rnd);
    Value c = crange(rnd);
    test_ZH_vs_Fit<NORMALIZE_EPD, NORMALIZE_ABC, EPD_SCALE, ABC_SCALE>(beta, p, u, c, maxDegree, maxSamples, maxItr, maxEval);
  }
}

/************************************************************************/
/*! \brief Compare ZH convolution evaluation versus fitted evaluation
  for non-unit b and lambda
*************************************************************************/
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc, bool EPD_SCALE=epd_scale, bool ABC_SCALE=abc_scale, bool CosScale=cos_scale>
void test_ZH_vs_Fit_nonunit(Value beta, Value p, Value u, Value c, Value b, Value lambda, size_t maxDegree, size_t maxSamples, size_t maxItr, size_t maxEval)
{
  Value scale = 1.0;
  if constexpr (EPD_SCALE) scale *= p * Constants::InvPi() * bbm::rcp(bbm::tgamma(1.0 / p) * beta * beta);
  if constexpr (ABC_SCALE) scale *= (c-1.0) * Constants::InvPi(0.5);

  const Vec2d init(2.0 / scale, 0.5);

  // convolve EPD and ABC in ZH (get both unit and non-unit convolution)
  std::vector<Value> conv, convRef;
  for(size_t l=0; l < maxDegree; ++l)
  {
    auto epd = bbm_ZH_EPD<NORMALIZE_EPD>(l, beta, p);
    conv.push_back( epd * bbm_ZH_ABC<NORMALIZE_ABC>(l, u*b/lambda, c) * bbm::sqrt(Constants::Pi(4.0) / (2.0*l + 1.0)));
    convRef.push_back( epd * bbm_ZH_ABC<NORMALIZE_ABC>(l, u, c, b, lambda) * bbm::sqrt(Constants::Pi(4.0) / (2.0*l + 1.0)));
  }
  
  // sample
  std::vector<Value> samples;
  for(size_t s=0; s < maxSamples; ++s)
  {
    Value sinTheta = 1.0 - Value(s) / Value(maxSamples);
    Value cosTheta = bbm::sqrt(1.0 - sinTheta*sinTheta);
    samples.push_back( evalConv(conv, cosTheta) );
  }

  // fit
  Vec2d param(init);
  lossfunc<NORMALIZE_ABC, CosScale> loss(param, samples, c, scale);
  compass opt(loss, param, Vec2d(0.0,0.0), Vec2d(10.0, 50.0), Constants::Epsilon(), 1.0, 0.5, 2.0);

  size_t i=0;
  for(i=0; i < maxItr && !opt.is_converged(); ++i)
  {
    auto err = opt.step();
    //std::cerr << i << ": " << param[0] << ", " << param[1] << ", " << err << std::endl;
  }
  
  // print warning if not converged
  if(!opt.is_converged())
    std::cout << " WARNING: optimization not converged (error = " << loss() << ")" << std::endl;

  // compare
  std::cerr << "Fit(u=" << u << ", c=" << c << ", p=" << p << ", beta=" << beta << ") - loss=" << loss() << ", param=" << param << ", b=" << b << ", lambda=" << lambda << ": ";
  for(size_t d=0; d != maxEval; ++d)
  {
    Value cosTheta = 1.0 - Value(d) / Value(maxEval);
    auto ec = evalConv(convRef, cosTheta);
    auto ef = evalFit<NORMALIZE_ABC,CosScale>(cosTheta, param, c, b, lambda)*scale;
    std::cerr << "(" << ec << " vs " << ef << "), ";
  }
  std::cerr << std::endl;
}

//! \brief do maxTests ZH vs Fit
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc, bool EPD_SCALE=epd_scale, bool ABC_SCALE=abc_scale>
  void test_ZH_vs_Fit_nonunit(size_t maxTests, size_t maxDegree, size_t maxSamples, size_t maxItr, size_t maxEval)
{
  for(size_t t=0; t < maxTests; ++t)
  {
    Value beta = betarange(rnd);
    Value p = prange(rnd);
    Value u = urange(rnd);
    Value c = crange(rnd);
    Value b = brange(rnd);
    Value lambda = lambdarange(rnd);
    test_ZH_vs_Fit_nonunit<NORMALIZE_EPD, NORMALIZE_ABC, EPD_SCALE, ABC_SCALE>(beta, p, u, c, b, lambda, maxDegree, maxSamples, maxItr, maxEval);
  }
}

/************************************************************************/
int main(int argc, char** argv)
{
  // test parameters
  const size_t maxTests = 8;
  const size_t maxEval = 8;
  const size_t maxDegree = 100;
  const size_t maxSamples = 1000;
  const size_t maxItr = 2000;

  // Validate midpoint vs brute force for sensible ranges of the parameters.
  std::cerr << "***Brute force versus midpoint***" << std::endl;
  test_BF_vs_Midpoint(maxEval);
  //test_BF_vs_Midpoint(maxEval, 0.0029, 5.0, 0.05, 3.02);

  // Validate ZH convolution vs evaluation
  std::cerr << "***Direct Evaluation vs. ZH evaluation***" << std::endl;
  test_Eval_vs_ZH(maxTests, maxDegree, maxEval);
  //test_Eval_vs_ZH(0.0029, 5.0, 0.05, 3.02, maxDegree, maxEval);
  
  // Validate fitting (for b and lambda == 1)
  std::cerr << "***Fitting versus ZH evaluation***" << std::endl;
  test_ZH_vs_Fit(maxTests, maxDegree, maxSamples, maxItr, maxEval);
  //test_ZH_vs_Fit(0.0029, 5.0, 0.05, 3.02, maxDegree, maxSamples, maxItr, maxEval);
  
  // Validate fitting for non-unit b and lambda
  std::cerr << "***Fitting versus ZH evaluation for non-unit b and lambda***" << std::endl;
  test_ZH_vs_Fit_nonunit(maxTests, maxDegree, maxSamples, maxItr, maxEval);
  //test_ZH_vs_Fit_nonunit(0.0029, 5.0, 0.05, 3.02, 0.1, 0.5, maxDegree, maxSamples, maxItr, maxEval);
  
  return 0;
}
