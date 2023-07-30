/************************************************************************/
/*! \file normalization.cpp

  \brief Compare a brute force computation of the renormalization constant
  (sigma_rel / sigma_s) (Eq 19) from "A two-scale microfacet reflectance model
  combining reflection and diffraction", Holzschuch and Pacanowski [2017]:
  https://doi.org/10.1145/3072959.3073621) with the precomputation as
  implemented in the reference code and with BBMs precomputation.

  Note: we compare a simplified version of Eq. 19 (by dividing by \f$ b^2 (c-1) / 2\pi \f$:

  \f$ (1 + b^2 f^2)^{-\frac{c+1}{2}} \f$

  since this matches what supposedly the code for generating the
  precomputation does in the reference code provided by Holzschuch and
  Pacanowski.

  This test shows that:

  + The provided Holzschuch and Pacanowski's reference code does *not*
  generate the ctd_renormalization.h file.

  + The precomputed table does not store the simplified version of Eq. 19 but
  \f$ \frac{sigma_{rel}^2}{sigma_s^2} \f$ computed with lambda set to 1.

  + Note that 'f' in Eq. 19 is defined with respect to lambda.  To include
  lambda in the evaluation, b is divided by lambda.  However, this introduces
  an error, since the normalization contains a \f$ b^2 \f$ without a
  corresponding \f$ f^2 \f$, and thus evaluating the precomputed table with
  embedding lambda in b, actually yields \f$ \frac{ \sigma_{rel}^2 }{
  \sigma_s^2 \lambda^2} \f$.
  
*************************************************************************/

#include <iostream>
#include <random>

#include "bbm.h"
using namespace bbm;
BBM_IMPORT_CONFIG(floatRGB)

// random number generator
std::mt19937 rnd;

/************************************************************************/
/*! \brief Generate a random Vec2d
*************************************************************************/
inline Vec2d rndVec2d(Value lower=-1, Value upper=+1)
{
  std::uniform_real_distribution<Value> U(lower, upper);
  return Vec2d(U(rnd), U(rnd));
}

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
/*! \brief Evaluate S_SH (based on Eq 16): \f$ (1+b^2 f^2)^{-\frac{c+1}{2}} /f$
*************************************************************************/
inline Value evalS_SH(const Value& f2, const Value& b2, const Value& c)
{
  return bbm::pow(1 + b2*f2, -(c+1)*0.5);
}

/************************************************************************/
/* \brief Compute sigma^2_rel / sigma^2_s using a brute force sampling of Eq 19.

   \param in = incident vector
   \param b2 = the squared 'b' parameter
   \param c = 'c' parameter
   \param samples = number of MC samples to compute integral
   \returns Integral of (1 + b^2f^2)^(-(c+1)/2)
   
   We assume that lambda is 1 (or alternatively that b is divided by lambda).
*************************************************************************/
inline Value brute_force(const Vec3d& in, const Value& b, const Value& c, size_t samples=1000000)
{
  auto rin = vec::xy(reflect(in));
  auto b2 = b*b;
  
  Value integral = 0;
  for(size_t s=0; s < samples; ++s)
  {
    auto out = rndVec2d(-1,+1);
    integral += bbm::select(bbm::squared_norm(out) < 1, evalS_SH(bbm::squared_norm(rin - out), b2, c), 0);
  }

  return integral / Value(samples) * 4.0;
}

/************************************************************************/
/*! \brief Precompute sigma^2_rel / sigma^2_s using the method of Holzschuch-Pacanowski

  \param sinTheta = sin theta of the incident angle
  \param b2 = the square of the 'b' parameter
  \param c = the 'c' parameter
  \returns Integral of (1 + b^2f^2)^(-(c+1)/2)

  We assume that lambda is 1 (or alternatively that b is divided by lambda).
  Note: The original code passes lambda^2 / b^2; the necessary adjustments
  have been made; otherwise the code remains the same.
**************************************************************************/
inline Value pre_sigma(const Value& sinTheta, const Value& b2, const Value& c)
{
  Value liminf = 1 - sinTheta;
  Value limsup = 1 + sinTheta;

  Value dv = 0.001 * Constants::Pi() / 180.0;
  Value result = 0;
  for(Value v = liminf; v < limsup; v += dv)
  {
    Value r2 = v*v;
    Value r = v;
    Value cos_thetaM = bbm::clamp((1 - r2 - sinTheta*sinTheta) / (2 * r * sinTheta), -1, +1);
    result += 2 * (Constants::Pi() - bbm::acos(cos_thetaM)) * v * dv * evalS_SH(v*v, b2, c);
  }
  return result;
}

/************************************************************************/
/*! \brief Out-of-cache computation of sigma^2_rel / sigma^2_s from the
    reference mitsuba BRDF implementation of Holzschuch-Pacanowksi (excluding
    b^2(c-1)/2PI factor).
    
    \param sinTheta = sin theta of the incident angle
    \param b2 = the square of the 'b' parameter
    \param c = the 'c' parameter
    \returns Integral of (1 + b^2f^2)^(-(c+1)/2)
*************************************************************************/
inline Value hp_sigma(const Value& sinTheta, const Value& b2, const Value& c)
{
  Value liminf = 1 - sinTheta;
  Value limsup = 1 + sinTheta;

  Value dv = 0.01 * Constants::Pi() / 180.0;
  Value result = (1 - bbm::pow(1 + liminf*liminf*b2, -0.5*(c-1))) * Constants::Pi(2) / (c-1) / b2;
  for(Value v = liminf; v < limsup; v += dv)
  {
    Value r2 = v*v;
    Value r = v;
    Value cos_thetaM = bbm::clamp((1 - r2 - sinTheta*sinTheta) / (2 * r * sinTheta), -1, +1);
    result += 2 * (Constants::Pi() - bbm::acos(cos_thetaM)) * v * dv * evalS_SH(v*v, b2, c);
  }
  return result;
}

    
/************************************************************************/
/*!

  S_SH is a radially symmetric function around (0,0).  The integral sigma_rel
  integrates over a disc centered around refl(in_xy).  Due to the radial
  symmetry, this is the same as a disc around in_xy. The radius of the disc is
  unit length, since the corresponding out-vector should have unit length.
  When |out_xy| <= 1, out_z can be computed such that |out| < 1.

  Lets denote the integration cirle by 'I', with radius 1.  Due to the
  symmetry of S_Sh we can without loss of generalization, rotate the space
  such that the in-vector lies in the X-Z plane, and thus the center of 'I'
  lies at (sin(theta_in), 0), and that sin(theta_in) >= 0.

  The iso lines of S_SH for some given f' lie on a circle 'M' centered at
  (0,0) with radius f'.

  We can now reduce the integration problem from a 2D integration (over x and
  y) to an integration over f' and the isolines (the latter which can be
  computed analytically).

  Given an f', the integral over the iso-line is S_SH(f') * \alpha * f, where
  \alpha = ArcLength of M inside I.  To compute the \alpha, we need to compute
  the intersection of M and I:

  \f$
    I(x,y): 0 = (x-\sin\theta_i)^2 + y^2 - 1
    M(x,y): 0 = x^2 + y^2 - f'^2
  \f$
  
  Rewrite in terms of y^2:

  \f$
    y^2 = 1 - (x-\sin\theta_i)^2
    y^2 = f'^2 - x^2
  \f$

  Substitute y^2:

  \f$
    f'^2 - x^2 = 1 - (x^2 - 2 x \sin\theta_i + \sin^2\theta_i)
    f'^2 = 1 + 2 x \sin\theta_i - \sin^2\theta_i
    x = (f'^2 - 1 + \sin^2\theta_i) / (2 \sin\theta_i)
  \f$
  
  Thus (x, sqrt(f'^2-x^2)) is the intersection.

  When |x/f'| > 1, then I lies inside M, or M lies inside I. The former can be
  avoided by limiting f' (the radius of M) to be less then \sin\theta_i.

  Note that \cos(\alpha/2) = x/f' when |x/f'| < 1, and f' > 0.

  For a range of the integral, the arclength \alpha = 2\pi, and the integral
  can be computed analytically. Consider that I is centered at (sinTheta,0)
  and has a radius of one. Thus, the point closest to the origin lies at
  1-\sin\theta_i, which is the maximum radius for which the arclength \alpha
  equals 2\pi. Rewriting the integral in polar coordinates yields:

  \f$
    \int_{2\pi} \int (1 + b^2 f^f)^{-\frac{c+1}{2}} f df d\theta =
    -2\pi \frac{(b^2 f^2 + 1)^\frac{1-c}{2}}{b^2 (c-1)}
  \f$

  Thus for the range (0, 1-sinTheta) this becomes:

  \f$
    \frac{2\pi}{b^2 (c-1)} ( 1 - (1 + b^2 (1-\sin\theta_i)^2))^\frac{c+1}{2}
  \f$
  
**************************************************************************/
inline Value bbm_sigma(const Value& sinTheta, const Value& b2, const Value& c)
{
  Value df = 0.001 * Constants::Pi() / 180.0;
  Value integral = Constants::Pi(2) / (b2 * (c-1)) * (1 - bbm::pow(1+b2*bbm::pow(1-sinTheta,2), 0.5*(1-c)));
  
  for(Value f=1-sinTheta; f <= sinTheta+1; f+=df)
  {
    auto x = (f*f - 1.0 + sinTheta*sinTheta) / (2.0*sinTheta);
    auto alpha = 2*bbm::safe_acos(x/f);
   integral += alpha * df * f * evalS_SH(f*f, b2, c);
  }
  
  return integral;
}


int main(int argc, char** argv)
{
  // Number of test
  size_t maxTests = 32;

  // Using some reasonable range
  std::uniform_real_distribution<Value> brange(0.001, 10.0);    
  std::uniform_real_distribution<Value> crange(1.1, 10.0);    
  
  // Do tests
  for(size_t t=0; t < maxTests; ++t)
  {
    // random in-dir
    Vec3d in = sampleHemisphere();

    // get b and c
    auto b = brange(rnd);
    auto c = crange(rnd);

    // eval
    auto bf = brute_force(in, b, c);
    auto pre = pre_sigma( spherical::sinTheta(in), b*b, c);
    auto hp = hp_sigma( spherical::sinTheta(in), b*b, c);
    auto bs = bbm_sigma( spherical::sinTheta(in), b*b, c);

    // Display results
    std::cout << "in = " << in << ", b = " << b << ", c = " << c << ": ";
    std::cout << bf << " vs. " << pre << " vs. " << hp << " vs. " << bs;

    Value n = b*b*(c-1)/Constants::Pi(2);
    std::cout << " sigma2_rel/sigma2_s = (" << bf*n << " vs. " << pre*n << " vs. " << hp*n << " vs. " << bs*n << ")" << std::endl;
  }

  // Done.
  return 0;
}
