#ifndef _BBM_BSDF_FLAGS_H_
#define _BBM_BSDF_FLAGS_H_

#include <array>
#include "util/flags.h"

/***********************************************************************/
/*! \file bsdf_flag.h
    \brief Reflectance component flags

    bsdf_flag::None
    bsdf_flag::Diffuse
    bsdf_flag::Specular
    bsdf_flag::All
************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief Reflectance Component Evaluation Flags
   *********************************************************************/
  enum struct bsdf_flag
  {
    None     = 0x0000,
    Diffuse  = 0x0001,
    Specular = 0x0002,
    All      = Diffuse | Specular
  };

  static constexpr std::array all_bsdf_flag{bsdf_flag::None, bsdf_flag::Diffuse, bsdf_flag::Specular, bsdf_flag::All};
  
  /////////////////////
  // ostream support //
  /////////////////////
  std::ostream& operator<<(std::ostream& s, const bbm::bsdf_flag& flag)
  {
    switch(flag)
    {
      case bbm::bsdf_flag::None :     { s << "None";     break; }
      case bbm::bsdf_flag::Diffuse :  { s << "Diffuse";  break; }
      case bbm::bsdf_flag::Specular : { s << "Specular"; break; }
      case bbm::bsdf_flag::All :      { s << "All";      break; }
    }
    return s;
  }
} // end bbm namespace

#endif /* _BBM_BSDF_FLAGS_H_ */
