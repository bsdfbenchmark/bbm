/************************************************************************/
/*! \file validate_convolution.cpp

  \brief Validate the precomputed convolutions and fits.
*************************************************************************/

#include <iostream>
#include <random>
#include <vector>

#include "bbm.h"
#include "precomputed/holzschuchpacanowski/convolution.h"
using namespace bbm;

BBM_IMPORT_CONFIG(floatRGB);

// random number generator
std::mt19937 rnd;

// parameter distributions
std::uniform_real_distribution<Value> urange(0, 1.0);          // can be much larger than 1.
std::uniform_real_distribution<Value> crange(1.1, 3.02);
std::uniform_real_distribution<Value> betarange(Constants::Epsilon(), 0.058);
std::uniform_real_distribution<Value> prange(0.05, 5);
std::uniform_real_distribution<Value> brange(0.01, 100.0);
std::uniform_real_distribution<Value> lambdarange(0.380, 0.780);

// Config
constexpr bool normalize_epd = true;     // normalize EPD computations
constexpr bool normalize_abc = false;    // normalize ABC computations
constexpr bool cos_scale = true;        // experimental

// Compare against HP's reference code?
#define INCLUDEHP

// param type
using namespace bbm::precomputed::holzschuchpacanowski;

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
/*! \brief Eq. 39;
*************************************************************************/
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc, bool CosScale=cos_scale>
inline Value evalFit(Value cosTheta, Value beta, Value p, Value u, Value c, Value b=1, Value lambda=1)
{
  Value normalization = (NORMALIZE_EPD) ? p * Constants::InvPi() * bbm::rcp(bbm::tgamma(1.0 / p) * beta * beta) : 1.0;
  auto param = (CosScale) ? bbm::get<"value">(convolution_cos.interpolate(b*u/lambda,c,p,beta)) : bbm::get<"value">(convolution.interpolate(b*u/lambda,c,p,beta));
  
  auto c2 = cosTheta*cosTheta;
  auto s2 = 1.0 - c2;
  auto f2 = 4.0 * bbm::get<"u">(param)*bbm::get<"u">(param) * s2 / (b*b);
  auto eval = bbm::get<"scale">(param) * S_HS<NORMALIZE_ABC>(f2, c, b) * normalization;
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
/*! \brief Compute the ZH coefficients for a given set of parameters
*************************************************************************/
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc>
  inline std::vector<Value> ZHcoefficients(size_t maxDegree, Value beta, Value p, Value u, Value c, Value b=1, Value lambda=1)
{
  std::vector<Value> conv;
  for(size_t l=0; l < maxDegree; ++l)
    conv.push_back( bbm_ZH_EPD<NORMALIZE_EPD>(l, beta, p) * bbm_ZH_ABC<NORMALIZE_ABC>(l, u, c, b, lambda) * bbm::sqrt(Constants::Pi(4.0) / (2.0*l + 1.0)));
  return conv;
}


/************************************************************************/
/*! \brief Compute error between Fit and ZH convolution
*************************************************************************/
template<bool NORMALIZE_EPD=normalize_epd, bool NORMALIZE_ABC=normalize_abc, bool CosScale=cos_scale>
  inline Value error(const std::vector<Value>& conv, Value beta, Value p, Value u, Value c, Value b=1, Value lambda=1, size_t samples=1000)
{


  Value err=0;
  for(size_t s=0; s < samples; ++s)
  {
    Value sinTheta = 1.0 - Value(s) / Value(samples);
    Value cosTheta = bbm::sqrt(1.0 - sinTheta*sinTheta);

    Value ez = evalConv(conv, cosTheta);
    Value ef = evalFit<NORMALIZE_EPD,NORMALIZE_ABC,CosScale>(cosTheta, beta, p, u, c, b, lambda);
    err += bbm::pow(ez - ef, 2.0);
  }

  // Done; return RMSE.
  return bbm::sqrt(err / Value(samples));
}

/***********************************************************************/
#ifdef INCLUDEHP

#include "ctd_convolution2.h"

const float bMax[98] = {0.058, 0.059, 0.059, 0.06, 0.06, 0.06, 0.061, 0.061, 0.062, 0.062, 0.063, 0.063, 0.064, 0.064, 0.065, 0.065, 0.066, 0.066, 0.067, 0.068, 0.068, 0.069, 0.07, 0.07, 0.071, 0.072, 0.073, 0.074, 0.074, 0.075, 0.076, 0.077, 0.078, 0.079, 0.081, 0.082, 0.083, 0.084, 0.084, 0.085, 0.085, 0.086, 0.086, 0.087, 0.087, 0.088, 0.089, 0.089, 0.09, 0.091, 0.092, 0.092, 0.093, 0.094, 0.095, 0.096, 0.098, 0.099, 0.1, 0.102, 0.104, 0.106, 0.108, 0.11, 0.113, 0.116, 0.119, 0.123, 0.127, 0.132, 0.138, 0.145, 0.153, 0.163, 0.175, 0.191, 0.211, 0.236, 0.268, 0.308, 0.348, 0.388, 0.416, 0.441, 0.461, 0.472, 0.47, 0.469, 0.448, 0.418, 0.389, 0.327, 0.261, 0.195, 0.121, 0.055, 0.015, 0.001};

float bMaxValue(const float p)
{
  if (p >= 5.0) return bMax[0]; 
  if (p <= 0.15) return bMax[97]; 
  float k_p = (5.0 - p) * 20.0; 
  int i_p = floor(k_p); 
  if (i_p < 0) return bMax[0]; // In theory  useless. Only there for floating point reasons
  if (i_p >= 97) return bMax[97];
  float a_p = k_p - i_p; 
  return (1. - a_p) * bMax[i_p] + a_p * bMax[i_p + 1]; 
}

template<bool NORMALIZE_EPD=normalize_epd>
  float computeConvolution(float beta, float p, float u, float c, float &scale, bool &isDirac) 
{
  scale = (NORMALIZE_EPD) ? p * Constants::InvPi() * bbm::rcp(bbm::tgamma(1.0 / p) * beta * beta) : 1.0;
  const float beta_betaMax = beta / bMaxValue(p);
  const float i_b = 10.0 * beta_betaMax - 0.5;
  int i_min = floor(i_b);
  int i_max = i_min + 1;
  float alpha_b = i_b - i_min;
  if (i_min < 0) i_min = 0;
  if (i_max < 0) {i_max = 0; alpha_b = 1.0;};
  if (i_max >= 9) i_max = 9;
  if (i_min >= 9) { i_min = 9; alpha_b = 0.0;}

  const float j_p = (5.0 - p) * 5;
  int j_min = floor(j_p);
  int j_max = j_min + 1;
  float alpha_p = j_p - j_min;
  if (j_min < 0) j_min = 0;
  if (j_max < 0) {j_max = 0; alpha_p = 1.0; }
  if (j_max >= 24) j_max = 24;
  if (j_min >= 24) { j_min = 24; alpha_p = 0.0;}
  
  const float k_c = 5.0 * (3.02 - c);
  int k_min = floor(k_c);
  int k_max = k_min + 1;
  float alpha_c = k_c - k_min;
  if (k_min < 0) k_min = 0;
  if (k_max < 0) {k_max = 0; alpha_c = 1.0; }
  if (k_max >= 10) k_max = 10;
  if (k_min >= 10) { k_min = 10; alpha_c = 0.0;}
  
  float u0[8];
  float a0[8];
  
  double index_u;
  int u_min;
  int u_max;
  double intervalWidth = 0.05;
  double uu = u;
  if (uu < 0.05) uu = 0.05;
  if (uu >= 100) uu = 100;
  if (uu <= 1) {
    intervalWidth = 0.05;
    index_u = uu / intervalWidth;
  } else if (uu <= 10) {
    intervalWidth = 1.0;
    index_u = (uu - 1.0)/intervalWidth + 20;
  } else if (uu <= 50) {
    intervalWidth = 5.0;
    index_u = (uu - 10.0)/intervalWidth + 29;
  } else if (uu <= 100) {
    intervalWidth = 10.0;
    index_u = (uu - 50.0)/intervalWidth + 37;
  }
  u_min = floor(index_u);
  u_max = u_min + 1;
  if (u_max > 42) u_max = 42;
  double alpha_u = index_u - u_min;
  u0[0] = (1. - alpha_u) *  convolutionFactors[j_min][i_min][k_min][u_min].uprime + alpha_u * convolutionFactors[j_min][i_min][k_min][u_max].uprime;
  u0[1] = (1. - alpha_u) *  convolutionFactors[j_min][i_min][k_max][u_min].uprime + alpha_u * convolutionFactors[j_min][i_min][k_max][u_max].uprime;
  u0[2] = (1. - alpha_u) *  convolutionFactors[j_max][i_min][k_min][u_min].uprime + alpha_u * convolutionFactors[j_max][i_min][k_min][u_max].uprime;
  u0[3] = (1. - alpha_u) *  convolutionFactors[j_max][i_min][k_max][u_min].uprime + alpha_u * convolutionFactors[j_max][i_min][k_max][u_max].uprime;
  u0[4] = (1. - alpha_u) *  convolutionFactors[j_min][i_max][k_min][u_min].uprime + alpha_u * convolutionFactors[j_min][i_max][k_min][u_max].uprime;
  u0[5] = (1. - alpha_u) *  convolutionFactors[j_min][i_max][k_max][u_min].uprime + alpha_u * convolutionFactors[j_min][i_max][k_max][u_max].uprime;
  u0[6] = (1. - alpha_u) *  convolutionFactors[j_max][i_max][k_min][u_min].uprime + alpha_u * convolutionFactors[j_max][i_max][k_min][u_max].uprime;
  u0[7] = (1. - alpha_u) *  convolutionFactors[j_max][i_max][k_max][u_min].uprime + alpha_u * convolutionFactors[j_max][i_max][k_max][u_max].uprime;

  a0[0] = (1. - alpha_u) *  convolutionFactors[j_min][i_min][k_min][u_min].scale + alpha_u * convolutionFactors[j_min][i_min][k_min][u_max].scale;
  a0[1] = (1. - alpha_u) *  convolutionFactors[j_min][i_min][k_max][u_min].scale + alpha_u * convolutionFactors[j_min][i_min][k_max][u_max].scale;
  a0[2] = (1. - alpha_u) *  convolutionFactors[j_max][i_min][k_min][u_min].scale + alpha_u * convolutionFactors[j_max][i_min][k_min][u_max].scale;
  a0[3] = (1. - alpha_u) *  convolutionFactors[j_max][i_min][k_max][u_min].scale + alpha_u * convolutionFactors[j_max][i_min][k_max][u_max].scale;
  a0[4] = (1. - alpha_u) *  convolutionFactors[j_min][i_max][k_min][u_min].scale + alpha_u * convolutionFactors[j_min][i_max][k_min][u_max].scale;
  a0[5] = (1. - alpha_u) *  convolutionFactors[j_min][i_max][k_max][u_min].scale + alpha_u * convolutionFactors[j_min][i_max][k_max][u_max].scale;
  a0[6] = (1. - alpha_u) *  convolutionFactors[j_max][i_max][k_min][u_min].scale + alpha_u * convolutionFactors[j_max][i_max][k_min][u_max].scale;
  a0[7] = (1. - alpha_u) *  convolutionFactors[j_max][i_max][k_max][u_min].scale + alpha_u * convolutionFactors[j_max][i_max][k_max][u_max].scale;

  float interpol_u0[4];
  float interpol_a0[4];
  interpol_u0[0] = (1. - alpha_c) * u0[0] + alpha_c * u0[1];
  interpol_a0[0] = (1. - alpha_c) * a0[0] + alpha_c * a0[1];
  interpol_u0[1] = (1. - alpha_c) * u0[2] + alpha_c * u0[3];
  interpol_a0[1] = (1. - alpha_c) * a0[2] + alpha_c * a0[3];
  interpol_u0[2] = (1. - alpha_c) * u0[4] + alpha_c * u0[5];
  interpol_a0[2] = (1. - alpha_c) * a0[4] + alpha_c * a0[5];
  interpol_u0[3] = (1. - alpha_c) * u0[6] + alpha_c * u0[7];
  interpol_a0[3] = (1. - alpha_c) * a0[6] + alpha_c * a0[7];

  float interpol_u1[2];
  float interpol_a1[2];
  interpol_u1[0] = (1 - alpha_p) * interpol_u0[0] + alpha_p * interpol_u0[1];
  interpol_u1[1] = (1 - alpha_p) * interpol_u0[2] + alpha_p * interpol_u0[3];
  interpol_a1[0] = (1 - alpha_p) * interpol_a0[0] + alpha_p * interpol_a0[1];
  interpol_a1[1] = (1 - alpha_p) * interpol_a0[2] + alpha_p * interpol_a0[3];

  scale *= (1 - alpha_b) * interpol_a1[0] + alpha_b * interpol_a1[1];
  float uprime  = (1. - alpha_b) * interpol_u1[0] + alpha_b * interpol_u1[1];

  if ((fabs((u - uprime)) < 0.01) && (fabs((scale - 1.0)) < 0.01)) {
    isDirac = true;
    scale = 1.0;
    return u;
  } else isDirac = false;
  return uprime;
}

template<bool NORMALIZE_EPD=normalize_epd>
  inline Value evalFit_HP(Value cosTheta, Value beta, Value p, Value u, Value c, Value b=1, Value lambda=1)
{
  bool dirac;
  float scale;
  float uprime = computeConvolution<NORMALIZE_EPD>(beta, p, b*u/lambda, c, scale, dirac);
  
  auto c2 = cosTheta*cosTheta;
  auto s2 = 1.0 - c2;
  auto f2 = 4.0 * uprime*uprime * s2 / (b*b);
  auto eval = scale * S_HS<false>(f2, c, b);
  return eval;
}


#endif /* INCLUDEHP */

/************************************************************************/
int main(int argc, char** argv)
{
  size_t maxDegree=100;
  size_t maxTests=8;
  size_t maxEval=8;

  for(size_t t=0; t < maxTests; ++t)
  {
    Value beta = betarange(rnd);
    Value p = prange(rnd);
    Value u = urange(rnd);
    Value c = crange(rnd);
    Value b = 1.0; //brange(rnd);
    Value lambda = 1.0; //lambdarange(rnd);

    std::cout << t << ": u=" << u << ", c=" << c << ", p=" << p << ", beta=" << beta << ", b=" << b << ", lambda=" << lambda;

    auto param = (cos_scale) ? convolution_cos.interpolate(b*u/lambda,c,p,beta) : convolution.interpolate(b*u/lambda,c,p,beta);
    std::cout << " [" << bbm::get<"value", "scale">(param) << ", " << bbm::get<"value", "u">(param) << "] (" << bbm::get<"valid">(param) << ") ";

    #ifdef INCLUDEHP
    if constexpr (!cos_scale)
    {
      bool dirac;
      float scale;
      float uprime = computeConvolution<false>(beta, p, b*u/lambda, c, scale, dirac);
      std::cout << "HP=(" << scale << ", " << uprime << " (" << dirac << ")) "; 
    } 
    #endif
    
    auto conv = ZHcoefficients(maxDegree, beta, p, u, c, b, lambda);
    auto err = error(conv, beta, p, u, c, b, lambda);
    std::cout  << " - error = " << err << ": ";
    
    for(size_t e=0; e < maxEval; ++e)
    {
      Value cosTheta = 1.0 -  Value(e) / Value(maxEval);
      Value ez = evalConv(conv, cosTheta);
      Value ef = evalFit(cosTheta, beta, p, u, c, b, lambda);
      std::cout << "(" << ez << " vs. " << ef;

      #ifdef INCLUDEHP
      float eh = evalFit_HP(cosTheta, beta, p, u, c, b, lambda);
      std::cout << " vs. " << eh;
      #endif
      
      std::cout << "), ";
    }
    std::cout << std::endl;
  }
  
  return 0;
}
