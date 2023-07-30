#include <iostream>
#include <vector>
#include <string>
#include "bbm.h"
#include "util/macro_util.h"
#include "util/string_literal.h"

int main(int /*argc*/, char** /*argv*/)
{
  // Print general bbm and backbone information
  std::cerr << "BBM_NAME = '" << BBM_STRINGIFY(BBM_NAME) << "' using '" << BBM_STRINGIFY(BBM_BACKBONE) << "' backbone and compiled with ";
  #if !defined(BBM_PYTHON)
    std::cerr << "NO ";
  #endif
  std::cerr << "python support." << std::endl;

  // Additional Enoki information
  if constexpr (bbm::string_literal(BBM_STRINGIFY(BBM_BACKBONE)) == bbm::string_literal("enoki"))
  {
    std::cerr << "Includes ENOKI compiled with: ";
    
    #if !defined(ENOKI_AUTODIFF)
      std::cerr << "NO ";
    #endif
    std::cerr << "Autodiff, ";

    #if !defined(ENOKI_CUDA)
      std::cerr << "NO ";
    #endif
    std::cerr << "CUDA, ";
  
    #if !defined(ENOKI_PYTHON)
      std::cerr << "NO ";
    #endif
    std::cerr << "Python" << std::endl;
  }

  // Additional DrJIT information
  if constexpr (bbm::string_literal(BBM_STRINGIFY(BBM_BACKBONE)) == bbm::string_literal("drjit"))
  {
    std::cerr << "Includes DrJIT compiled with: ";

    #if !defined(DRJIT_JIT)
      std::cerr << "NO ";
    #endif
    std::cerr << "JIT, ";
    
    #if !defined(DRJIT_AUTODIFF)
      std::cerr << "NO ";
    #endif
    std::cerr << "Autodiff, ";

    #if !defined(DRJIT_PYTHON)
      std::cerr << "NO ";
    #endif
    std::cerr << "Python" << std::endl;
  }

  // print all models
  std::vector<std::string> bsdflist;
  #undef BBM_EXPORT_BSDFMODEL
  #define BBM_EXPORT_BSDFMODEL(bsdfmodel) bsdflist.push_back( bsdfmodel<bbm::floatRGB>::name.value );
  #include "bbm_bsdfmodel.h"
  #include "export/clear_export.h"

  std::cerr << bsdflist.size() << " BSDF models supported:" << std::endl;
  for(auto& m : bsdflist)
    std::cerr << " + " << m << std::endl;

  // print all static models
  std::vector<std::string> staticlist;
  #undef BBM_EXPORT_BSDFMODEL
  #define BBM_EXPORT_BSDFMODEL(staticmodel) staticlist.push_back( staticmodel<bbm::floatRGB>::name.value );
  #include "bbm_staticmodel.h"
  #include "export/clear_export.h"

  std::cerr << staticlist.size() << " static (measured) BSDFs supported:" << std::endl;
  for(auto& m : staticlist)
    std::cerr << " + " << m << std::endl;
  
  return 0;
}
