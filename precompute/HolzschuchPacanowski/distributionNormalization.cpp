#include <iostream>
#include <fstream>
#include <string>

/************************************************************************/
/*! \file distributionNormalization.cpp
  
  \brief Precompute the EPD normalization "A two-scale
  microfacet reflectance model combining reflection and diffraction",
  Holzschuch and Pacanowski [2017]: https://doi.org/10.1145/3072959.3073621

  We store\f$ \frac{p}{\pi \Gamma(1/p)} \f$ in 256 steps using the encoding by
  Holzschuch and Pacanowski:

  \f$
    index_p = p * 256.0 / 5.0
  \f$

*************************************************************************/

////////////////////
// Setup BBM code
////////////////////
#include "bbm_core.h"
using namespace bbm;
BBM_IMPORT_CONFIG( doubleRGB );

////////////////////////////////////
// Define the file header and tail
////////////////////////////////////
std::string_view header[] = {
  "#ifndef _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_DISTRIBUTION_NORMALIZATION_H_",
  "#define _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_DISTRIBUTION_NORMALIZATION_H_",
  "",
  "/************************************************************************/",
  "/* Precomputed EPD normalization (without beta) from 'A two-scale       */",
  "/* microfacet reflectance model combining reflection and diffraction',  */",
  "/* Holzschuch and Pacanowski [2017]:  doi.org/10.1145/3072959.3073621   */",
  "/************************************************************************/", 
  "",
  "namespace bbm {                                                           ",
  "  namespace precomputed {                                                 ",
  "    namespace holzschuchpacanowski {                                      ",
  "",
  "      const tab<float, std::array{256},                                   ",
  "           decltype( [](const auto& p) { return p * 256.0 / 5.0; } )      ",
  "         > distributionNormalization = {                                  "
};

std::string_view tail[] = {
  "      };                                                                  ",
  "    } // end holzschuchpacanowski namespace                               ",
  "  } // end precomputed namespace                                          ",
  "} // end bbm namespace                                                    ",
  "",
  "#endif /* _BBM_PRECOMPUTE_HOLZSCHUCHPACANOWSKI_DISTRIBUTION_NORMALIZATION_H_ */"
};


int main(int argc, char** argv)
{
  /////////////////
  // Parse input
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
  const size_t p_size = 256;
  for(size_t p_index = 0; p_index < p_size; ++p_index)
  {
    // show percentage
    std::cout << "\r" << (100 * p_index / p_size) << "% precomputed" << std::flush;

    // compute normalization
    Value p = 5.0 * Value(p_index) / Value(p_size); 
    Value val = bbm::select(p_index == 0, 0.0, p * Constants::InvPi() * bbm::rcp( bbm::tgamma(1.0 / p) ) );

    // write out precomputed value
    if(p_index % 8 == 0) ofs << "        ";
    ofs << val;
    if(p_index+1 != p_size) ofs << ", ";
    if(p_index % 8 == 7) ofs << std::endl;
  }

  // write tail
  for(auto& t : tail)
    ofs << t << std::endl;

  // Done (clear percentage print).
  std::cout << "\r" << std::flush;

  return 0;
}

