#ifndef _BBM_BSDF_FLAGS_H_
#define _BBM_BSDF_FLAGS_H_

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

  BBM_ENUM(bsdf_flag, None, Diffuse, Specular, All);
  
} // end bbm namespace

#endif /* _BBM_BSDF_FLAGS_H_ */
