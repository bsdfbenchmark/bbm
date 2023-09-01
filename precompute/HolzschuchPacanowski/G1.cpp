#include <iostream>
#include <fstream>
#include <string>

/************************************************************************/
/*! \file G1.cpp

  \brief Precompute G1 from: "A two-scale microfacet reflectance model
  combining reflection and diffraction", Holzschuch and Pacanowski [2017]:
  https://doi.org/10.1145/3072959.3073621

  We store the precomputed values using the following transformations:
  * p_idx = 5.0 / p - 1;
  * tanTheta_idx = return bbm::exp(-bbm::pow(bbm::log(bbm::rcp(t)), 0.05)) * 1000.0 - 1.0;
  This is the same encoding as Holzschuch and Pacanowski.

  Notes on the 0.05 exponent.

  G1 (and it's sister equation P2) are integral from some lower bound to
  +infinity.  Plese refer to the methods below for more detailed discussion on
  each of these functions. In order to sample this range, a change of
  variables is applied:

  \f$
    x' = log^\gamma(1/x)
  \f$

  In theory, an exponent of \f$ \gamma = 1 \f$ should work.  However, we will
  use a midpoint approximation over a 1000 intervals.  This means that in
  practice the integral is computed from 0 to ~6.9, which clearly is not
  [0,+infinity].  Applying an exponent > 1 extends the range (as well as
  places more samples in 0...1).
  
  Emperical testing has shown that an exponent of at most p works in most
  cases for computing G1 and P2.  Since the smallest p for which G1 is
  precomputed is 0.05, this is also the upper bound for the exponent.
  
*************************************************************************/

///////////////////
// Setup BBM core
///////////////////
#include "bbm_core.h"
using namespace bbm;
BBM_IMPORT_CONFIG( floatRGB );

/************************************************************************/
/*! \brief Compute P2: Eq 36

  Note: Eq 36 in the paper has a typo: the lower bound of the integral should
  be minus infinity, not zero.

  \f$
    P_2(r) = N \int_{-\infty}^{+\infty} e^{-(r^2 + q^2)^p} dq,
  \f$

  where N is a normalization factor:

  \f$
    N = \frac{p}{\pi \Gamma(1/p)}.
  \f$
  
  To numerically compute this integral, we employ:
  1) a midpoint approximation
  2) use an exponentiated log encoding of 1/x, with x in [0...1].

  The mid point approximation is given by:

  \f$
    P_2(r) \approx N \sum_i \delta_q e^{-(r^2 + q_i^2)^p}
  \f$

  where $\f$ \delta_q = b_i - a_i \f$ and \f$ a_i = b_{i-1} \f$ and \f$ q_i
  \in [a_i, b_i] \f$.

  We use the following encoding for a and b:

  \f$
    b = \log^\gamma(\frac{1}{x})
  \f$

  where \f$ \gamma = 20 \f$ emperically choosen to provide both a dense
  sampling around 0 and towards sufficient large values. x in [0...1] sampled
  in 1000 steps.  Note this will compute the integral from 0..infinity, thus
  we scale the integral by two (the integrant is symmetric around 0).

  Normally, q = 0.5 (a+b). However, due to the exponential nature of the
  integrant, we found that placing q in the middle of the x domain between to
  corresponding sample values (in x) of a and b works better.
***********************************************************************/
inline Value P2(Value r, Value p, Value gamma=20)
{
  const Value normalization = p / (Constants::Pi() * bbm::tgamma(1.0 / p));
  const Value r2 = r*r;
  const Value deltax = 0.0001;

  // compute exponentiated log encoding
  auto conv = [&](auto x) { return bbm::pow(bbm::log(bbm::rcp(x)), gamma); };

  // approximate with a midpoint method for a 1000 intervals
  Value integral = 0;
  for(Value x=1; bbm::cast<bool>(x > deltax); x -= deltax)
  {
    Value delta_q = conv(x - deltax) - conv(x);
    Value q = conv(x - 0.5*deltax);

    if(bbm::none(bbm::isnan(delta_q))) integral += delta_q * bbm::exp(-bbm::pow(r2 + q*q, p));
  }

  // The above integral is from 0...+infinity. The function is symmetric,
  // hence, we just double the result.
  return 2.0 * normalization * integral;
}


/************************************************************************/
/*! \brief Precompute G1 for a series of tan(theta).

  G1 is defined in function of p and a \f$ \beta\tan\theta \f$ (we assume \f$
  \beta = 1 \f$ as this can be easily multiplied in later):

  \f$
    \Delta(\beta\tan\theta) = \inf_\frac{1}{\beta\tan\theta}^\infty (r*\beta\tan\theta - 1) P_2(r) dr
  \f$

  with \f$ G1 = \frac{1}{1 + \Delta} \f$. Similar to P2, we will compute this
  with a midpoint approximation and an exponentiated log encoding of
  \f$ \beta\tan\theta \f$.

  \f$
    \Delta(\beta\tan\theta) = \sum_{i=\frac{1}{\beta\tan\theta}}^\infty \delta_r (r\beta\tan\theta - 1) P_2(r,p)
  \f$

  We encode:

  \f$
    r = \log^\gamma(\frac{1}{x})
  \f$

  where x [0, T], and

  \f$
    T = e^{-(\frac{1}{\beta\tan\theta})^\frac{1}{\gamma}}
  \f$

  Note: \f$ \log^\gamma(\frac{1}{T}) == \frac{1}{\beta\tan\theta} \f$ the
  lower bound of the integral (and 0 will be mapped to +infinity).

  Next consider that we want to compute this integral for different values of
  T (and thus \f$ \beta\tan\theta \f$). We will therefore regularly sample T
  in steps of 0.001, denote the j-th sample as \f$ T_j \f$, then:

  \f$
    \beta\tan\theta_j = \frac{1}{ \log^\gamma(\frac{1}{T_j}) }
  \f$

  The choice of encoding (identical to the encoding of 'r' in the integrant)
  is deliberate in order to compute \f$ \Delta(\cdot) \f$ incrementally with
  maximum reuse of computations.  To derive the incremental computation, we
  formulate \f$ \Delta_j = \Delta(\beta\tan\theta_j) \f$ as:

  \f$
    \Delta_j = \sum_{i=0}^T_j \delta_{r_i} (r_i \beta\tan\theta_j - 1) P_2(r_i),
             = \sum_i \delta_{r_i} r_i \beta\tan\theta_j P_2(r_i) - sum_i \delta_{r_i} P_2(r_i),
             = \sum_i \delta_{r_i} r_i \beta\tan\theta_j P_2(r_i) - P_j
             = (\Delta_{j-1} + P_{j-1}) \frac{\tan\theta_j}{\tan\theta_{j-1}} + \delta_{r_j} r_j \beta\tan\theta_j P_2(r_j) - (P_{j-1} + \delta_{r_j} P_2(r_j))
             = ((\Delta_{j_1} + P_{j-1}) \frac{\tan\theta_j}{\tan\theta_{j-1}} - P_{j-1}) + (\delta_{r_j} (r_j \beta\tan\theta_j - 1) P_2(r_j))
  \f$

  Thus we can compute \f$ \Delta_j from \tan\theta_{j-1}, \Delta_{j-1} and
  P_{j-1} \$f and a single evaluatation of P2.

*************************************************************************/
inline auto G1series(const Value& p, const Value& gamma=20)
{
  constexpr size_t size = 1000;
  const Value delta_x = bbm::rcp(Value(size));

  // exponentiated log encoding
  auto conv = [&](auto x) { return bbm::pow(bbm::log(bbm::rcp(x)), gamma); };
  
  // create result
  std::array<Value, size> integral;

  // compute each entry in the array
  integral[0] = 0;
  Value prevTanTheta = 0;
  Value P_j = 0;
  
  for(size_t j=1; j < size; ++j)
  {
    std::cerr << j << ", ";
    
    Value x = (j+1) / Value(size);      // == T ( we drop the 'j' index)
    Value tanTheta = bbm::rcp(conv(x));

    if(bbm::none(bbm::isinf(tanTheta)))           // theta != grazing angle
    {
      Value delta_r = conv(x - delta_x) - conv(x);
      Value r = conv(x - 0.5*delta_x);
      Value p2 = P2(r, p) * delta_r;    // \f$ P_2(r_j) \f$

      integral[j] = 0;
      if(bbm::any(prevTanTheta > 0)) integral[j] = (integral[j-1] + P_j) * tanTheta / prevTanTheta - P_j;  // 1st term
      if(bbm::any(r*tanTheta > 1)) integral[j] += (r*tanTheta-1)*p2;  // 2nd term

      // save for next step
      prevTanTheta = tanTheta;
      P_j += p2;
    
      if(bbm::any(bbm::isnan(integral[j])))
        throw std::runtime_error("UNEXPECTED");
    }
    else integral[j] = tanTheta;        // theta is at grazing angle => +infinity
  }

  // Convert Delta to G1
  for(auto& i : integral)
    i = 1.0 / (1.0 + i);

  // Done.
  return integral;
}


////////////////////////////////////
// Define the file header and tail
////////////////////////////////////
std::string_view header[] = {
  "#ifndef _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_G1_H_                       ",
  "#define _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_G1_H_                       ",
  "",
  "/************************************************************************/",
  "/* Precomputed undirectional shawowing term G1 from 'A two-scale        */",
  "/* microfacet reflectance model combining reflection and diffraction',  */",
  "/* Holzschuch and Pacanowski [2017]:  doi.org/10.1145/3072959.3073621   */",
  "/************************************************************************/", 
  "",
  "namespace bbm {                                                           ",
  "  namespace precomputed {                                                 ",
  "    namespace holzschuchpacanowski {                                      ",
  "",
  "      static const tab<float, std::array{100,1000},                       ",
  "           decltype( [](const auto& p) { return 5.0 / p - 1.0; } ),       ",
  "           decltype( [](const auto& t) { return bbm::exp(-bbm::exp(bbm::log(bbm::rcp(t)) * 0.05)) * 1000.0 - 1.0; } ) ",
  "         > G1 = {                                                         "
};

std::string_view tail[] = {
  "      };                                                                  ",
  "    } // end holzschuchpacanowski namespace                               ",
  "  } // end precomputed namespace                                          ",
  "} // end bbm namespace                                                    ",
  "",
  "#endif /* _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_G1_H_ */                   "
};

int main(int argc, char** argv)
{
  ////////////////
  // parse input
  ////////////////
  if(argc == 1)
  {
    std::cout << "Usage: " << argv[0] << " <header file name>" << std::endl;
    return -1;
  }

  std::string filename(argv[1]);

  // open file
  std::ofstream ofs(filename, std::ios::out);
  if(!ofs.is_open()) throw std::runtime_error(std::string("BBM: ") + argv[0] + " failed to open: " + filename);

  // write header
  for(auto& h : header)
    ofs << h << std::endl;

  // precompute G1
  for(size_t p_idx = 0; p_idx < 100; ++p_idx)
  {
    Value p = 5.0 / Value(p_idx + 1);
    
    // show percentage
    std::cout << "\r" << p_idx << "% precomputed" << std::flush;
    
    // write comment in file for readability
    ofs << "        // p = " << p << std::endl;
    ofs << "        ";
    
    // compute G1 for the full series of tan(theta)
    auto integral = G1series(p);

    // write out precomputed values for G1
    for(size_t i=0; i < integral.size(); ++i)
    {
      ofs << bbm::toString( integral[i] );
      if(p_idx+1 != 100 || i+1<integral.size()) ofs << ", ";
    }
    ofs << std::endl;
  }
  
  // write tail
  for(auto& t : tail)
    ofs << t << std::endl;
  
  // Done.
  return 0;
}


