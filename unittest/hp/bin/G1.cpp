/************************************************************************/
/*! \file P2.cpp

  \brief Compare a brute force computation of P2 (Eq 36) from "A two-scale
  microfacet reflectance model combining reflection and diffraction",
  Holzschuch and Pacanowski [2017]: https://doi.org/10.1145/3072959.3073621)
  with the precomputation as implemented in the reference code and with BBMs
  precomputation.

*************************************************************************/

#include <iostream>
#include <random>

#include "bbm.h"
using namespace bbm;
BBM_IMPORT_CONFIG(floatRGB);

// random number generator
std::mt19937 rnd;

/************************************************************************/
/*! \brief Generate point uniformly on hemisphere (pdf = 1/2pi)
*************************************************************************/
Vec3d sampleHemisphere(void)
{
  std::uniform_real_distribution<Value> U(0,1);
  Vec2d coord;
  spherical::theta(coord) = bbm::safe_acos(U(rnd));
  spherical::phi(coord) = U(rnd) * Constants::Pi(2);

  return spherical::convert(coord);
}

/************************************************************************/
/*! \brief Brute force computation of Eq 36 (note: the integral should go from
    -infinity (not zero) to +infinity
*************************************************************************/
inline Value brute_force_P2(const Value& r, const Value& p, size_t samples=10000)
{
  auto normalization = p / (Constants::Pi() * bbm::tgamma(1.0 / p));
  auto r2 = r*r;

  // integrate for 0->infinity; due to sqaure of q; the integral is just double
  std::uniform_real_distribution<Value> range(0, 1);
  
  Value integral = 0;
  for(size_t s=0; s < samples; ++s)
  {
    auto xi = range(rnd);
    auto q = bbm::log(bbm::rcp(xi)); 
    auto dq = bbm::rcp(xi);

    if(!bbm::isinf(q) && !bbm::isnan(dq))
      integral += bbm::exp(-bbm::pow(r2 + q*q, p)) * dq / Value(samples);
  }

  // Done.
  return 2.0 * integral * normalization;
}

/************************************************************************/
/*! \brief Brute force computation of 37 and 38
*************************************************************************/
inline Value brute_force_G1(const Vec3d& in, const Value& beta, const Value& p, size_t samples=1000)
{
  // compute Eq 37
  auto bTanTheta = beta * spherical::tanTheta(in);

  auto limit = bbm::exp(-bbm::rcp(bTanTheta));
  std::uniform_real_distribution<Value> range(0, limit);

  Value delta = 0;
  for(size_t s=0; s < samples; ++s)
  {
    auto xi = range(rnd);
    auto r = bbm::log(bbm::rcp(xi));
    auto dr = bbm::rcp(xi);

    if(!bbm::isnan(dr) && !bbm::isinf(r))
      delta += (r*bTanTheta-1.0)*brute_force_P2(r,p) * dr * limit / samples;
  }

  // compute Eq 38
  return 1.0 / (1.0 + delta);
}

/************************************************************************/
/*! \brief bbm precomputation of P2: using midpoint inetegration
/************************************************************************/
inline Value bbm_P2(const Value& r, const Value& p, const Value& exponent=1)
{
  auto normalization = p / (Constants::Pi() * bbm::tgamma(1.0 / p));
  auto r2 = r*r;
  auto deltax = 0.0001;

  auto conv = [&](auto x) { return bbm::pow(bbm::log(bbm::rcp(x)), exponent); };
  
  Value integral = 0;
  for(Value x=1; x > deltax; x -= deltax)
  {
    Value dq = conv(x-deltax) - conv(x);
    Value q = conv(x-0.5*deltax);
    
    if(!bbm::isnan(dq)) integral += dq * bbm::exp(-bbm::pow(r2 + q*q, p));
  }

  return 2.0 * integral * normalization;
}


/************************************************************************/
/*! \brief BBM precomputation computation of 37 and 38
*************************************************************************/
inline Value bbm_G1(const Vec3d& in, const Value& beta, const Value& p, Value exponent=0)
{
  if(exponent == 0) exponent = bbm::max(1.0, 1.0/p);

  // compute Eq 37
  auto bTanTheta = beta * spherical::tanTheta(in);
  auto conv = [&](auto x) { return bbm::pow(bbm::log(bbm::rcp(x)), exponent); };

  auto limit = bbm::exp(-bbm::pow(bbm::rcp(bTanTheta), 1.0 / exponent));
  auto deltax = 0.001;

  Value integral = 0;
  for(Value x=limit; x > deltax; x -= deltax)
  {
    auto dr = conv(x-deltax) - conv(x);
    Value r = conv(x-0.5*deltax);

    if(!bbm::isnan(dr))
      integral += (r*bTanTheta-1.0)*bbm_P2(r,p,exponent) * dr;
  }

  // compute Eq 38
  return 1.0 / (1.0 + integral);
}

/************************************************************************/
/*! \brief Reimplementation of Holzschuch and Pacanowski's reference code

  Using the midpoint rule for numerical integration.
*************************************************************************/
inline Value hp_P2(const Value& r, const Value& p)
{
  auto normalization = p / (Constants::Pi() * bbm::tgamma(1.0 / p));
  auto r2 = r*r;
  Value dkt = 0.0001;
  Value exponent = 20.0;
  
  Value integral = 0;
  Value oldv = 0;
  for(Value kt = 1; kt > 0; kt -= dkt)
  {
    Value v = bbm::pow(bbm::log(1/kt), exponent);
    Value eval = bbm::exp(-bbm::pow(r2 + v*v, p));
    Value deltav = bbm::pow(bbm::log(1/(kt-dkt)), exponent) - oldv;
    integral += bbm::select(bbm::isnan(deltav), 0, eval * deltav);
    oldv = v;
  }

  return integral * normalization;
}


/************************************************************************/
/*! \brief HP computation of 37 and 38
  
  Midpoint variant.  s0 is computed while in the reference code it comes from
  a different source (all is embedded in one giant double loop to compute all
  G1 values).
  
*************************************************************************/
inline Value hp_G1(const Vec3d& in, const Value& beta, const Value& p)
{
  auto t = beta*spherical::tanTheta(in);
  if(t == 0) return 1;

  Value exponent = 20.0;
  auto ustart = 1.0 / t;

  Value lambda = 0;
  auto oldu = ustart;
  auto s0 = (1.0 - bbm::exp(-bbm::pow(oldu, 1.0/exponent))) * 1000.0;
  for(size_t s=s0; s < 1000; ++s)
  {
    Value ks = 1.0 - s/1000.0;
    Value u = bbm::pow(bbm::log(1/ks), exponent);
    Value nextks = 1.0 - (s+1)/1000.0;
    Value nextu = bbm::pow(bbm::log(1/nextks), exponent);
    if(bbm::isinf(nextu)) nextu = u;
    Value deltau = 0.5 * (nextu - oldu);
    Value value = hp_P2(u, p);
    lambda += deltau * (t*u - 1) * value;
    oldu = u;
  }

  return 1.0 / (1.0 + lambda);
}


/************************************************************************/
/*! \brief Test the impact of the exponent in compressing the sampling range
    in computing P2.
*************************************************************************/
inline void exponentTestP2(size_t maxExp=14)
{
  const std::array parray{0.05, 0.075, 0.1, 0.5, 1.0, 2.0, 5.0};
  
  for(const Value& p: parray)
  {
    std::cout << "P2: p = " << p << ": ";
    for(Value exponent=1; exponent < maxExp; ++exponent)
    {
      Value bm = bbm_P2(0, p, exponent);
      std::cout << "(" << exponent << ") = " << bm << ", ";
    }
    std::cout << std::endl;
  }
}

/************************************************************************/
/*! \brief Test the impact of the exponent in compressing the sampling range
    in computing G1.
*************************************************************************/
inline void exponentTestG1(size_t maxTests, size_t maxExp=20)
{
  const std::array parray{0.05, 0.075, 0.1, 0.5, 1.0, 2.0, 5.0};

  for(size_t t=0; t < maxTests; ++t)
  {
    Vec3d in = sampleHemisphere();
    std::cout << "in = " << in << std::endl;
    
    for(const Value& p: parray)
    {
      std::cout << "G1: p = " << p << ": ";
      for(Value exponent : std::array{1.0/p, 1.0, 10.0, 20.0})
      {
        Value bm = bbm_G1(in, 1, p, exponent);
        std::cout << "(" << exponent << ") = " << bm << ", ";
      }
      std::cout << std::endl;
    }
  }
}

/************************************************************************/
/*! \brief Test P2 computation. When p=1 an analytical solution can be
    computed:

    \f$
    P_2(r) = \int_{-\infty}^\infty e^{-(r^2 + q^2)^p} dq, 
           = \int_{-\infty}^infty e^{-r^2} e^{-q^{2} dq, with p = 1
           = e^{-r^2} \int_{-\infty}^infry e^{-q^2} dq,
           = \frac { e^{-r^2} }{\sqrt{\pi}}
    \f$
*************************************************************************/    
inline void testP2_p1(size_t maxTests)
{
  // Using some reasonable range
  std::uniform_real_distribution<Value> rrange(0, 10);
  const Value p = 1;
  
  for(size_t t=0; t < maxTests; ++t)
  {
    auto r = rrange(rnd);
    auto bf = brute_force_P2(r, p);
    auto hp = hp_P2(r, p);
    auto ref = bbm::exp(-r*r) * Constants::InvSqrtPi();
    auto bm = bbm_P2(r, p);
    
    std::cout << "P2: r = " << r << ", p = " << p << ": " << ref << " vs. (hp) " << hp << " vs. (bf) " << bf << " vs. " << bm << std::endl;
  }

  // Done.
}

/************************************************************************/
/*! \brief Test P2 computation. p randomly choosen 
*************************************************************************/    
inline void testP2(size_t maxTests)
{
  // Using some reasonable range
  std::uniform_real_distribution<Value> rrange(0, 3);
  std::uniform_real_distribution<Value> prange(0.05, 5);
  
  for(size_t t=0; t < maxTests; ++t)
  {
    auto r = rrange(rnd);
    auto p = prange(rnd);
    auto bf = brute_force_P2(r, p);
    auto hp = hp_P2(r, p);
    auto bm = bbm_P2(r, p);
    
    std::cout << "P2: r = " << r << ", p = " << p << ": (hp) " << hp << " vs. (bf) " << bf << " vs. " << bm << std::endl;
  }

  // Done.
}

/************************************************************************/
/*! \brief Test G1 computation. When p=1, G1 corresponds to the Beckman G1
    solution. We will use beta=1 (no stretch)
*************************************************************************/
inline void testG1_p1(size_t maxTests)
{
  const Value beta = 1;
  const Value p = 1;

  ndf::beckmann<Config, symmetry_v::Isotropic, false> beckmann(beta);
  
  for(size_t t=0; t < maxTests; ++t)
  {
    Vec3d in = sampleHemisphere();
    auto ref = beckmann.G1(in, Vec3d(0,0,1));
    auto bf = brute_force_G1(in, beta, p);
    auto hp = hp_G1(in, beta, p);
    auto bm = bbm_G1(in, beta, p);
    
    std::cout << "G1: in = " << in << ", beta = " << beta << ", p = " << p << ": " << ref << " vs. (hp) " << hp << " vs. (bf) " << bf << " vs. " << bm << std::endl;
  }

  // Done.
}

/************************************************************************/
/*! \brief Test G1 computation. p randomly choosen, We will use beta=1 (no
    stretch)
*************************************************************************/
inline void testG1(size_t maxTests)
{
  const Value beta = 1;
  std::uniform_real_distribution<Value> prange(0.05, 5);

  for(size_t t=0; t < maxTests; ++t)
  {
    auto p = prange(rnd);
    Vec3d in = sampleHemisphere();
    auto bf = brute_force_G1(in, beta, p);
    auto hp = hp_G1(in, beta, p);
    auto bm = bbm_G1(in, beta, p);
    
    std::cout << "G1: in = " << in << ", beta = " << beta << ", p = " << p << ": (hp) " << hp << " vs. (bf) " << bf << " vs. " << bm << std::endl;
  }

  // Done.
}

/************************************************************************/

int main(int argc, char** argv)
{
  // Number of tests
  size_t maxTests = 16;

  //exponentTestP2();
  //exponentTestG1(maxTests);
  
  // Test P2 computation
  testP2_p1(maxTests);
  std::cout << std::endl;

  testP2(maxTests);
  std::cout << std::endl;
  
  // Test G1 computation
  testG1_p1(maxTests);
  std::cout << std::endl;

  testG1(maxTests);
  std::cout << std::endl;
  
  // Done.
  return 0;
}
