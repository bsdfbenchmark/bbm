#include <iostream>
#include <fstream>
#include <string>

/************************************************************************/
/*! \file normalization.cpp
 
  \brief Precompute the renormalization ratio from: "A two-scale microfacet
  reflectance model combining reflection and diffraction", Holzschuch and
  Pacanowski [2017]: https://doi.org/10.1145/3072959.3073621

  Computes \f$ \frac{ \sigma_{rel}^2 }{ sigma_s^2 \lambda^2} as the integral
  over \f$ S_SH \f$ for the three variables: \$ \sin\theta_i,
  \frac{b^2}{\lambda^2}, c \f$.
  
*************************************************************************/

///////////////////
// Setup BBM core
//////////////////
#include "bbm_core.h"
using namespace bbm;
BBM_IMPORT_CONFIG( floatRGB );

/************************************************************************/
/*!

  Precompute:

  \f$
    \frac{\sigma_rel^2}{\sigma_s^2} = \int_{|o|<1} \frac{S_HS(f, b, c)}{\sigma_s^2} d f_x d f_y
  \f$

  for various values of \f$ \sin\theta_i \f$, b, and c, where:
  
  \f$
    \frac{S_HS(f, b, c)}{\sigma_s^2} = \frac{b^2(c-1)}{2\pi} (1 + b^2 f^2)^{-\frac{c+1}{2}},
  \f$

  and

  \f$
    f = ||\frac{1}{\lamba}((i + o) \times n) \times n || = \frac{1}{\lambda} || (i+o)_{xy} ||. 
  \f$

  For ease of notation we will denote this function simply by S_HS.
  Furthermore, we will assume for precomputations that f (which depends on the
  wavelength \f$ \lambda \f$) is given for \f$ \lambda = 1 \f$.  Note,
  however, that \f$ \sigma_s^2 \f$ is integrated over \f$ f \f$, and thus
  independent of wavelength! \f$ \sigma_rel^2 \f$ should be dependent on
  wavelength (it integrates for \f$ |out| < 1 \f$ and thus $\lambda$ is rolled
  in the integration domain).  Therefore, to lookup a precomputed value for a
  non-unit wavelength, we can divide b with \f$ \lambda \f$ (which in the
  denominator of the equation is multiplied with f and thus scales f
  appropriately).  This has the added advantage that the \f$ b^2 \f$ factor in
  the normalization of S_HS now becomes \f$ b^2 / \lambda^2 \f$, and thus the
  evaluation of the precomputated are therefore correctly accounting for
  wavelength.

  To precompute consider the following (assuming unit b and wavelength):
  
  S_HS is a radially symmetric function around (0,0).  The integral sigma_rel
  integrates over a disc centered around \f$ refl(in_xy) \f$.  Due to the
  radial symmetry, this is the same as a disc around \f$ in_xy \f$. The radius
  of the (integration) disc is unit length, since the corresponding out-vector
  should have unit length.  When \f$ |out_xy| <= 1, out_z \f$ can be computed
  such that \f$ |out| < 1 \f$.

  Lets denote the integration cirle by 'I', with radius 1.  Due to the
  symmetry of S_HS we can without loss of generalization, rotate the space
  such that the in-vector lies in the X-Z plane, and thus the center of 'I'
  lies at (sin(theta_in), 0), and that sin(theta_in) >= 0.

  The iso lines of S_HS for some given f' lie on a circle 'M' centered at
  (0,0) with radius f'.

  We can now reduce the integration problem from a 2D integration (over x and
  y) to an integration in polar coordinates over f' and the isolines (the
  latter which can be computed analytically).

  Given an f', the integral over the iso-line is \f$ S_HS(f') * \alpha, \f$
  where \alpha = ArcLength of M inside I.  To compute the \alpha, we need to
  compute the intersection of M and I:

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

  Thus by Monte Carlo integration over 'f' we can estimate the integral by:

  \f$
    \alpha * f * S_HS(f, b, c)
  \f$
  
  For a range of the integral, the arclength \alpha = 2\pi, and the integral
  can be computed analytically. Consider that I is centered at (sinTheta,0)
  and has a radius of one. Thus, the point closest to the origin lies at
  1-\sin\theta_i, which is the maximum radius for which the arclength \alpha
  equals 2\pi. Rewriting the integral in polar coordinates yields:

  \f$
    \int_{2\pi} \int frac{b^2 (c-1)}{2\pi} (1 + b^2 f^f)^{-\frac{c+1}{2}} f df d\theta =
    -\frac{(1 + b^2 f^2)^\frac{1-c}{2}}{b^2 (c-1)}
  \f$

  Thus for the range (0, 1-sinTheta) this becomes:

  \f$
    ( 1 - (1 + b^2 (1-\sin\theta_i)^2))^\frac{c+1}{2}
  \f$
  
**************************************************************************/
inline Value integralSH(const Value& sinTheta, const Value& b2, const Value& c)
{
  // lambda for computing S_HS (Eq. 16, without normalization)
  auto S_HS = [&](auto& f) { return bbm::pow(1 + b2*f*f, -(c+1)*0.5); };

  // integral f = [1-sinTheta ... 1+sinTheta]
  Value integral = 0; 
  Value df = 0.01 * Constants::Pi() / 180.0;
  for(Value f=1-sinTheta; f <= sinTheta+1; f+=df)
  {
    auto x = (f*f - 1.0 + sinTheta*sinTheta) / (2.0*sinTheta);
    auto alpha = 2*bbm::safe_acos(x/f);
    integral += alpha * f * df * S_HS(f);
  }

  // include normalization of Eq. 16
  integral *= (b2*(c-1)) * Constants::InvPi(0.5);
  
  // integral f = [0...1-sinTheta]
  integral += (1 - bbm::pow(1+b2*bbm::pow(1-sinTheta,2), 0.5*(1-c)));

  // Done.
  return integral;
}


///////////////////////////////////////////
// Define the header file header and tail
///////////////////////////////////////////
std::string_view header[] = {
  "#ifndef _BBM_PRECOMPUTE_HOLZSCHUCHPACAKNOWSKI_NORMALIZATION_H_           ",
  "#define _BBM_PRECOMPUTE_HOLZSCHUCHPACAKNOWSKI_NORMALIZATION_H_           ",
  "",
  "/************************************************************************/",
  "/* Precomputed renormalization factor sigma_rel^2/sigma_s^2             */",
  "/* from 'A two-scale microfacet reflectance model combining reflection  */",
  "/* and diffraction', Holzschuch and Pacanowski [2017]:                  */",
  "/* doi.org/10.1145/3072959.3073621                                      */",
  "/************************************************************************/", 
  "",
  "namespace bbm {                                                           ",
  "  namespace precomputed {                                                 ",
  "    namespace holzschuchpacanowski {                                      ",
  "",
  "      const tab<float, std::array{100,100,100},                           ",
  "           decltype( [](const auto& b) { return 101.0*(10.0/(b+10.0))-1.0; } ),",
  "           decltype( [](const auto& c) { return 101.0*(c-1)/c-1.0; } ),   ",
  "           decltype( [](const auto& sinTheta) { return sinTheta*100.0; } )",
  "         > renormalization = {                                            "
  };

std::string_view tail[] = {
  "      };                                                                  ",
  "    } // end holzschuchpacanowski namespace                               ",
  "  } // end precomputed namespace                                          ",
  "} // end bbm namespace                                                    ",
  "",
  "#endif /* _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_NORMALIZATION_H_ */        "
};

int main(int argc, char** argv)
{
  /////////////////
  // parse input //
  /////////////////
  if(argc == 1)
  {
    std::cout << "Usage: " << argv[0] << " <header file name>" << std::endl;
    return -1;
  }

  std::string filename(argv[1]);

  std::ofstream ofs(filename, std::ios::out);
  if(!ofs.is_open()) throw std::runtime_error(std::string("BBM: ") + argv[0] + " failed to open: " + filename);
  
  // write header
  for(auto& h : header)
    ofs << h << std::endl;

  // precompute
  const size_t samples = 100;
  for(size_t bIndex=0; bIndex != samples; ++bIndex)
  {
    // inv_b = 1 / b
    auto inv_b = 0.1 * (Value(bIndex)+1) / (Value(samples) - Value(bIndex));

    // show percentage precomputed (this will take a while)
    std::cout << "\r" << bIndex << "% precomputed" << std::flush;
    
    for(size_t cIndex=0; cIndex != samples; ++cIndex)
    {
      auto c = (Value(samples)+1) / (Value(samples) - Value(cIndex));

      // write comment in file to make it more readable
      ofs << "        // b = " << bbm::rcp(inv_b) << ", c = " << c << std::endl;
      ofs << "        ";
      
      for(size_t sinThetaIndex=0; sinThetaIndex != samples; ++sinThetaIndex)
      {
        auto sinTheta = Value(sinThetaIndex) / Value(samples);
        auto integral = integralSH(sinTheta, bbm::rcp(inv_b*inv_b), c);

        // write out precomputed value
        ofs << integral;
        if(bIndex+1 != samples || cIndex+1 != samples || sinThetaIndex+1 != samples) ofs << ", ";
      }
      
      ofs << std::endl;
    }
  }
  
  // write tail
  for(auto& t : tail)
    ofs << t << std::endl;
  
  // Done (clear percentage print).
  std::cout << "\r" << std::flush;
  
  return 0;
}
