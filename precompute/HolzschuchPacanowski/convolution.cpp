#include <iostream>
#include <fstream>
#include <string>

/************************************************************************/
/*! \file convolution.cpp
  
  \brief Precompute the convolution between S and the NDF: "A two-scale
  microfacet reflectance model combining reflection and diffraction",
  Holzschuch and Pacanowski [2017]: https://doi.org/10.1145/3072959.3073621

  The precomputation operates in two stages:

  1. compute the innerproduct of Zonal Harmonics (upto degree 100) and S and
  the NDF. Finally, compute the innerproduct as the product of both
  terms. (Appendix A, Eqs. 61-64).

  2. fit two K-correlation models (s and g) to the convolution, and store the
  data in a lookup table. (Eqs. 39-41):

  \f$
    (S * D)(\theta_h) \approx s(u, c, \beta, p) S_{HS}(f') \cos^2\theta_h
    u = \frac{b}{\lambda} \cos\theta_d
    f' = \frac{2}{b} \sin\theta_h g(u, c, \beta, p)
  \f$

  Note: we found that adding a \f$ \cos^2\theta_h \f$ greatly improves fitting
  accuracy.  This makes sense, since S does include this term, and D is often
  close to a dirac, thus the convolution is a scaled version of (a slightly
  blurred) S. For backward compatibility, we compute a version with the
  additional cos scaling, and one without.
  
  Hence, we store all data (two scalars; the evaluation of s and g) in a 4D
  lookup table.  We follow the encoding of Holzschuch and Pacanowski:

  \f$
    index_p = 25 - 5p,                                p \in [0.2, ..., 5],                        \delta_p = 0.2
    index_c = 5 (3.02 - c),                           c \in [3.02, ..., 1.02],                    \delta_c = 0.2
    index_\beta = 10\frac{\beta}{\beta_{max}} - 0.5,  \frac{\beta}{\beta_{max}} \in [0.05, 0.95], \delta_\beta = 0.1
    \index_u & = & u / \delta_u,                      if u \leq 1,                                \delta_u = 0.05
             & = & (u-1) / \delta_u + 20,             if 1 < u \leq 10,                           \delta_u = 1
             & = & (u-10) / \delta_u + 29,            if 10 < u < \leq 50,                        \delta_u = 5
             & = & (u-50) / \delta_u + 37,            if 50 < u \leq 100,                         \delta_u = 10
  \f$
  
  The precomputed table (u,c,p,\beta) has size (43,11,25,10).  \f$ \beta_{max}
  \f$ directly copied from Holzschuch and Pacanowski to best mimic the results
  from the reference code; no real pattern was discovered in how this table is
  created.  Note: the order of precomputed values differs from Holzschuch and
  Pacanowski's reference code.
  
  Another key difference with the reference code from Holzschuch and
  Pacanowski is that we employ a direct search algorithm (compass search) to
  fit s and g instead of a Levenberg-Marquard non-linear optimization.
  Furthermore, we reuse the solution of previous optimizations as a starting
  point.  Similar to Holzschuch and Pacanowski, we do not include the
  normalization factors in the fitted model, but expect these to be factored
  in when evaluation.  A small difference to Holzschuch and Pacanowski's
  reference code is that we do no normalize the EPD during evaluation and
  fitting, since including (or excluding) this factor does not affect the
  parameter fitting (it only costs computation time).

  Finally, note that the precomputation generator provided by Holzschuch and
  Pacanowski is not the one used to generated the ctd_convolution.h file
  provided with the reference code.
  
  Summary of differences versus the reference code of Holzschuch and Pacanowski:
  - Do not normalize twice by EPD factor when fitting (see fitZonalHarmonics)
  - Scale s*S_SH by a cos^2
  - Use compass search for optimizing.
  - Reuse prior optimizations as starting point
  - Results are stored in a different order
  - Compute and store in float instead of double.

*************************************************************************/

////////////////////
// Setup BBM core
////////////////////
#include "bbm_core.h"
#include "optimizer/compass.h"
using namespace bbm;
BBM_IMPORT_CONFIG( floatRGB );

constexpr size_t beta_size = 10;
constexpr size_t p_size = 25;
constexpr size_t c_size = 11;
constexpr size_t u_size = 43;

/************************************************************************/
/*! \brief Beta_max as defined by Holzschuch and Pacanowski.
*************************************************************************/
tab<float, std::array{25},
    decltype( [](const auto& p) { return (5.0 - p) * 5.0; } )
   > betamax = {
      0.058, 0.060, 0.062, 0.064, 0.066,       // p=5.0->4.2  
      0.068, 0.071, 0.074, 0.078, 0.083,       // p=4.0->3.2
      0.085, 0.087, 0.090, 0.093, 0.098,       // p=3.0->2.2
      0.104, 0.113, 0.127, 0.153, 0.211,       // p=2.0->1.2
      0.348, 0.461, 0.448, 0.261, 0.015        // p=1.0->0.2
};


/************************************************************************/
/*! @{ \name Coordinate to Index
*************************************************************************/
auto u_index = [](const auto& u, const auto&, const auto&, const auto&)
 {
   std::decay_t<decltype(u)> index = 0;
   auto mask = (u <= 1.0);
   if(bbm::any(mask)) index = bbm::select(mask, u / 0.05, index);
   mask = (u > 1) && (u <= 10);
   if(bbm::any(mask)) index = bbm::select(mask, u + 19.0, index);
   mask = (u > 10) && (u <= 50);
   if(bbm::any(mask)) index = bbm::select(mask, (u - 10.0) / 5.0 + 29.0, index);
   mask = (u > 50); 
   if(bbm::any(mask)) index = bbm::select(mask, (u-50.0) / 10.0 + 37.0, index);
   return index;
 };

auto c_index = [](const auto&, const auto& c, const auto&, const auto&)
 {
   return 5.0*(3.02 - c);
 };

auto p_index = [](const auto&, const auto&, const auto& p, const auto&)
 {
   return 25.0 - 5.0*p;
 };

auto beta_index = [](const auto&, const auto&, const auto& p, const auto& beta)
 {
   return 10.0 * beta / bbm::get<"value">(betamax.interpolate<std::decay_t<decltype(beta)>>(p)) - 0.5;
 };
//! @}            

/************************************************************************/
/*! @{ \name Index to Coord
/************************************************************************/
auto u_coord = [](const auto& u_index)
 {
   if(u_index > 37) return (u_index - 37.0)*10.0 + 50.0;
   else if(u_index > 29) return (u_index - 29.0)*5.0 + 10.0;
   else if(u_index > 19) return (u_index - 19.0)*1.0 + 0.0;
   else return u_index * 0.05;
 };

auto c_coord = [](const auto& c_index)
 {
   return 3.02 - c_index / 5.0;
 };

auto p_coord = [](const auto& p_index)
 {
   return 5.0 - p_index / 5.0;
 };

auto beta_coord = [](const auto& p_index, const auto& beta_index)
 {
   return (beta_index + 0.5) / 10.0 * betamax[p_index];
 };
//! @}


//! \brief Data type for storing the ZH convolutions
using zhconv_t = tab< std::vector<Scalar>, std::array{u_size,c_size,p_size,beta_size}, decltype(u_index), decltype(c_index), decltype(p_index), decltype(beta_index) >;

/************************************************************************/
/*! \brief Compute Zonal harmonics fits to the K-correlation model and to the
    Exponential Power Distribution: Eq 61-64

    To minimize evaluation of the Legendre Polynomals, we re-order the
    computations to first iterate over the degree 'l'.  We also postpone the
    normalzation to the evaluation of Eq. 64.

    The inner-product of the Zonal Harmonics and S (and D) is computed with
    the midpoint algorithm for integration (with max_samples intervals).
*************************************************************************/
inline zhconv_t precomputeZonalHarmonicsConvolution(size_t max_samples=600000, size_t max_degree=100)
{
  // D and S lambdas (without normalization)
  auto D_func = [](auto& cosTheta, auto& beta, auto& p)
  {
    auto c2 = cosTheta*cosTheta;
    auto s2 = 1.0 - c2; 
    return bbm::exp(-bbm::pow(s2 / (c2*beta*beta), p)) / (c2*c2);  // exp(- (tan2/beta2)^p ) / cos^4
  };

  auto S_func = [](auto& cosTheta, auto& u, auto& c)
  {
    auto c2 = cosTheta*cosTheta;
    auto s2 = 1 - c2;
    auto bf2 = 4.0 * u*u * s2; // bf = 2 (b/lambda cosTheta_d) sinTheta
    return bbm::pow(1.0+bf2, -0.5*(c+1.0)) * c2;
  };
  
  // ***Eq. 61-63*** => use Midpoint integration and store results in D_coef and S_coef
  tab< std::vector<Scalar>, std::array{p_size,beta_size}, decltype(p_index), decltype(beta_index) > D_coef;
  for(auto& d : D_coef) d = std::vector<Scalar>(max_degree);

  tab< std::vector<Scalar>, std::array{u_size,c_size}, decltype(u_index), decltype(c_index) > S_coef;
  for(auto& s : S_coef) s = std::vector<Scalar>(max_degree);

  Scalar delta_cosTheta = 1.0 / Scalar(max_samples);
  
  // for each legendre degree 'l' (thus for each Zonal Harmonics of degree 'l')
  for(size_t l=0; l < max_degree; ++l)
  {
    std::cout << "\r[1/3] Zonal Harmonics inner-product: " << (100*l/max_degree) << "% precomputed                              " << std::flush;

    // ZH normalization factor
    auto normalization = bbm::sqrt((2.0*l + 1.0) * Constants::InvPi(0.25));

    // midpoint integration sample scale factor
    normalization *= (Constants::Pi(2.0) * delta_cosTheta);
    
    // for each cosTheta (1 -> 0): evaluate the Zonal Harmonics and multiply with S or D.
    for(Scalar cosTheta=1; cosTheta > 0; cosTheta -= delta_cosTheta)
    {
      auto ZH = std::legendre(l, cosTheta) * normalization;

      // update inner-product of D and ZH
      for(size_t p_index=0; p_index < p_size; ++p_index)
        for(size_t beta_index=0; beta_index < beta_size; ++beta_index)
        {
          auto p = p_coord(p_index);
          auto beta = beta_coord(p_index, beta_index);
          D_coef(p_index, beta_index)[l] += D_func(cosTheta, beta, p) * ZH;
        }
      
      // update inner-product of S and ZH
      for(size_t u_index=0; u_index < u_size; ++u_index)
        for(size_t c_index=0; c_index < c_size; ++c_index)
        {
          auto u = u_coord(u_index);
          auto c = c_coord(c_index);
          S_coef(u_index, c_index)[l] += S_func(cosTheta, u, c) * ZH;
        }
    }
  } // end degree (l) loop

  // ***Eq. 64***
  zhconv_t convolution;
  for(auto& c : convolution) c = std::vector<Scalar>(max_degree);

  for(size_t l=0; l < max_degree; ++l)
  {
    for(size_t u_index=0; u_index < u_size; ++u_index)
      for(size_t c_index=0; c_index < c_size; ++c_index)
      {
        auto s = S_coef(u_index, c_index)[l];
        for(size_t p_index=0; p_index < p_size; ++p_index)
          for(size_t beta_index=0; beta_index < beta_size; ++beta_index)
            convolution(u_index,c_index,p_index,beta_index)[l] += s * D_coef(p_index, beta_index)[l];
      }
  }

  // Done.
  return convolution;
}

/************************************************************************/
/*! \brief Eval the convolution for all precomputed ZH terms and all sampled
    parameters.

    The evaluation happens for a regular sampling of sin(theta_h) because g'
    as defined in Eq. 41 is parameterized in function of sin(theta_h).
*************************************************************************/
inline zhconv_t evaluateZonalHarmonicsConvolution(const zhconv_t& convolution, size_t max_samples=1000, size_t max_degree=100)
{
  zhconv_t eval;
  for(auto& e : eval) e = std::vector<Scalar>(max_samples);
  
  // Precompute evaluation of the convolution
  Scalar delta_cosTheta = 1.0 / Scalar(max_samples);
  for(size_t l=0; l < max_degree; ++l)
  {
    std::cout << "\r[2/3] Evaluating Convolution: " << (100*l/max_degree) << "% precomputed                              " << std::flush;
    for(size_t s=0; s < max_samples; ++s)
    {
      Scalar sinTheta = 1.0 - Scalar(s) / Scalar(max_samples);
      Scalar cosTheta = bbm::sqrt(1.0 - sinTheta*sinTheta);

      // evaluate the unnormalized ZH at degree l and for cosTheta:
      auto P = std::legendre(l, cosTheta);

      // sum over all coefficients per degree
      for(size_t u_idx=0; u_idx < u_size; ++u_idx)
        for(size_t c_idx=0; c_idx < c_size; ++c_idx)
          for(size_t p_idx=0; p_idx < p_size; ++p_idx)
            for(size_t beta_idx=0; beta_idx < beta_size; ++beta_idx)
              eval(u_idx, c_idx, p_idx, beta_idx)[s] += convolution(u_idx, c_idx, p_idx, beta_idx)[l] * P;
    }
  }

  // Done.
  return eval;
}


//! \brief loss function for fitting ZH to K-correlation (meets concepts::lossfunction)
template<bool CosScale=false>
struct lossfunc
{
  BBM_IMPORT_CONFIG( floatRGB );

  lossfunc(const Vec2d& param, const std::vector<Scalar>& reference, Scalar c) : _param(param), _reference(reference), _c(c) {}
  void update(void) {}

  //! \brief Eval S_HS with normalization
  inline auto S(Scalar cosTheta, Scalar u, Scalar c) const
  {
    auto c2 = cosTheta*cosTheta;
    auto s2 = 1 - c2;
    auto bf2 = 4.0 * u*u * s2;
    auto eval = bbm::pow(1+bf2, -0.5*(c+1));
    if constexpr (CosScale) eval *= c2;
    return eval;
  }

  //! \brief Eval the loss
  Value operator()(Mask=true) const
  {
    Scalar err = 0;
    for(size_t s=0; s < _reference.size(); ++s)
    {
      auto sinTheta = 1.0 - Value(s) / Value(_reference.size());
      auto cosTheta = bbm::sqrt(1.0 - sinTheta*sinTheta);
      err += bbm::pow( _param[0] * S(cosTheta, _param[1], _c) - _reference[s], 2.0 );
    }
    return err;
  }
    
private:
  Scalar _c;
  const Vec2d& _param;
  const std::vector<Scalar>& _reference;
};
BBM_CHECK_RAW_CONCEPT( concepts::lossfunction, lossfunc<false> );


//! \brief Data type for storing the fitted parameters
using fit_t = tab< vec2d<float>, std::array{u_size,c_size,p_size,beta_size}, decltype(u_index), decltype(c_index), decltype(p_index), decltype(beta_index) >;


/************************************************************************/
/*! \brief Fit Eq 39-41 to the Zonal Harmonics coefficients of the
    convolution.

    Assumes b==1 and lambda==1.

    We assume the fitted parameters vary smoothly with varying sampled
    parameters. Therefore, we use the previous fit as a starting point for the
    next non-linear fit.

*************************************************************************/
template<bool CosScale=false>
  inline fit_t fitZonalHarmonics(const zhconv_t& evaluation, size_t max_samples=1000, size_t max_itr=4096)
{
  Scalar percentage = 0.0;
  Scalar delta = 100.0 / Scalar(evaluation.size());

  // set initial starting point
  Vec2d init(0.3, 0.5);
  
  // allocate result structure
  fit_t fit;

  // for each case:
  for(size_t u_idx=0; u_idx < u_size; ++u_idx)
  {
    auto u = u_coord(u_idx);
    for(size_t c_idx=0; c_idx < c_size; ++c_idx)
    {
      auto c = c_coord(c_idx);
      for(size_t p_idx=0; p_idx < p_size; ++p_idx)
      {
        auto p = p_coord(p_idx);
        for(size_t beta_idx=0; beta_idx < beta_size; ++beta_idx)
        {
          auto beta = beta_coord(p_idx, beta_idx);
          
          std::cout << "\r[3/3] Fitting (CosScale=" << CosScale << "): " << int(percentage) << "% precomputed                              " << std::flush;  percentage += delta;

          // get precompute ZH convolution evaluation
          const auto& eval = evaluation(u_idx, c_idx, p_idx, beta_idx);

          // Perform fit (without cos weighting)
          Vec2d param(init);
          lossfunc<CosScale> loss(param, eval, c);
          compass opt(loss, param, Vec2d(0.0,0.0), Vec2d(10.0, 50.0), Constants::Epsilon(), 1, 0.5, 2.0);

          size_t i=0;
          for(i=0; i < max_itr && !opt.is_converged(); ++i)
            auto err = opt.step();

          // print warning if not converged
          if(!opt.is_converged())
          {
            std::cout << " WARNING: optimization not converged (error = " << loss() << " init = " << init << ", u = " << u << ", c = " << c << ", beta = " << beta << ", p = " << p << ")" << std::endl;
            //exit(1);
          }
          
          // store
          fit(u_idx, c_idx, p_idx, beta_idx) = param;

          // use previous result as next init
          init = param;
        }
        
        // use previous result as next init
        init = fit(u_idx, c_idx, p_idx, 0);
      }
      
      // use previous result as next init
      init = fit(u_idx, c_idx, 0, 0);
    }

    // use previous result as next init
    init = fit(u_idx, 0, 0 ,0);
  }

  // Done.
  return fit;
}



////////////////////////////////////
// Define the file header and tail
////////////////////////////////////
std::string_view header[] = {
  "#ifndef _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_CONVOLUTION_H_              ",
  "#define _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_CONVOLUTION_H_              ",
  "",
  "/************************************************************************/",
  "/* Precomputed convolution between S and the NDF from 'A two-scale       */",
  "/* microfacet reflectance model combining reflection and diffraction',  */",
  "/* Holzschuch and Pacanowski [2017]:  doi.org/10.1145/3072959.3073621   */",
  "/************************************************************************/", 
  "",
  "namespace bbm {                                                           ",
  "  namespace precomputed {                                                 ",
  "    namespace holzschuchpacanowski {                                      ",
  "",
  "      const tab<float, std::array{25},                                    ",
  "           decltype( [](const auto& p) { return (5.0 - p) * 5.0; } )      ",
  "         > betamax = {                                                    ",
  "           0.058, 0.060, 0.062, 0.064, 0.066,       // p=5.0->4.2         ",
  "           0.068, 0.071, 0.074, 0.078, 0.083,       // p=4.0->3.2         ",
  "           0.085, 0.087, 0.090, 0.093, 0.098,       // p=3.0->2.2         ",
  "           0.104, 0.113, 0.127, 0.153, 0.211,       // p=2.0->1.2         ",
  "           0.348, 0.461, 0.448, 0.261, 0.015        // p=1.0->0.2         ",
  "      };                                                                  ",
  ""
};

std::string_view data_start[] = {
  "      const tab<named<std::tuple<float,float>, \"scale\", \"u\"> , std::array{43,11,25,10},  // (u,c,p,beta) ",
  "           decltype( [](const auto& u, const auto&, const auto&, const auto&)",
  "                     {                                                    ",
  "                       std::decay_t<decltype(u)> index = 0;               ",
  "                       auto mask = (u <= 1.0);                            ",
  "                       if(bbm::any(mask)) index = bbm::select(mask, u / 0.05, index);",
  "                       mask = (u > 1) && (u <= 10);                       ",
  "                       if(bbm::any(mask)) index = bbm::select(mask, u + 19.0, index);",
  "                       mask = (u > 10) && (u <= 50);                      ",
  "                       if(bbm::any(mask)) index = bbm::select(mask, (u - 10.0) / 5.0 + 29.0, index);",
  "                       mask = (u > 50);                                   ",
  "                       if(bbm::any(mask)) index = bbm::select(mask, (u-50.0) / 10.0 + 37.0, index);",
  "                       return index;                                      ",
  "                     } ),                                                 ",
  "           decltype( [](const auto&, const auto& c, const auto&, const auto&)",
  "                     {                                                    ",
  "                       return 5.0*(3.02 - c);                             ",
  "                     } ),                                                 ",
  "           decltype( [](const auto&, const auto&, const auto& p, const auto&)",
  "                     {                                                    ",
  "                        return 25.0 - 5.0*p;                              ",
  "                     } ),                                                 ",
  "           decltype( [](const auto&, const auto&, const auto& p, const auto& beta)",
  "                     {                                                    ",
  "                       return 10.0 * beta / bbm::get<\"value\">(betamax.interpolate<std::decay_t<decltype(beta)>>(p)) - 0.5;",
  "                     } )                                                  "
};

std::string_view name_nocos[] = {
  "          > convolution = {{{                                             "
};

std::string_view name_cos[] = {
  "          > convolution_cos = {{{                                         "
};

std::string_view data_end[] = {
  "      }}};                                                                ",
  ""
};

std::string_view tail[] = {
  "    } // end holzschuchpacanowski namespace                               ",
  "  } // end precomputed namespace                                          ",
  "} // end bbm namespace                                                    ",
  "",
  "#endif /* _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_CONVOLUTION_H_ */          "
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

  // precompute ZH inner product
  zhconv_t convolution = precomputeZonalHarmonicsConvolution();

  // evaluate the convolutions for a discrete set of samples
  zhconv_t evaluation = evaluateZonalHarmonicsConvolution(convolution);

  // fit s and g to the ZH convolution (Eq. 39-41)
  fit_t fit = fitZonalHarmonics<false>(evaluation);
  fit_t fit_cos = fitZonalHarmonics<true>(evaluation);

  // save result to file
  std::string filename(argv[1]);

  std::ofstream ofs(filename, std::ios::out);
  if(!ofs.is_open()) throw std::runtime_error(std::string("BBM: ") + argv[0] + " failed to open: " + filename);

  // write header
  for(auto& h : header)
    ofs << h << std::endl;

  // write data
  auto writeData = [&](const fit_t& fit) {
    for(size_t u_idx=0; u_idx < u_size; ++u_idx)
    {
      auto u = u_coord(u_idx);
      for(size_t c_idx=0; c_idx < c_size; ++c_idx)
      {
        auto c = c_coord(c_idx);
        for(size_t p_idx=0; p_idx < p_size; ++p_idx)
        {
          auto p = p_coord(p_idx);
        
          // output a new line per u,c,p to make it more readable
          ofs << "        // u = " << u << ", c = " << c << ", p = " << p << ", beta_max = " << betamax[p_idx] << std::endl;
          ofs << "        ";
        
          for(size_t beta_idx=0; beta_idx < beta_size; ++beta_idx)
          {
            auto beta = beta_coord(p_idx, beta_idx);
          
            //ofs << "// beta = " << beta << std::endl;
            auto ug = fit(u_idx, c_idx, p_idx, beta_idx);
            ofs << "{" << ug[0] << ", " << ug[1] << "}";
            if(u_idx+1 != u_size || c_idx+1 != c_size || p_idx+1 != p_size || beta_idx+1 != beta_size) ofs << ", ";
          }

          ofs << std::endl;
        }
      }
    }
  };

  // no cos
  for(auto& d : data_start)  ofs << d << std::endl;
  for(auto& n : name_nocos)  ofs << n << std::endl;
  writeData(fit);
  for(auto& e : data_end) ofs << e << std::endl;

  // cos
  for(auto& d : data_start)  ofs << d << std::endl;
  for(auto& n : name_cos)  ofs << n << std::endl;
  writeData(fit_cos);
  for(auto& e : data_end) ofs << e << std::endl;
  
  // write tail
  for(auto& t : tail)
    ofs << t << std::endl;

  // Done (clear percentage print)
  std::cout << "\r" << std::flush;

  return 0;
}
