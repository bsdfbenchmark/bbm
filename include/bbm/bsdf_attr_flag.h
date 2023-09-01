#ifndef _BBM_BSDF_ATTR_FLAG_H_
#define _BBM_BSDF_ATTR_FLAG_H_

#include "util/flags.h"

/***********************************************************************/
/*! \file bsdf_attr_flag.h
  \brief Flags to mark properties of a bsdf's attribute.
************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief Attribute Property Flags
   *********************************************************************/
  enum struct bsdf_attr
  {
    None                = 0x0000,
    DiffuseScale        = 0x0001,
    DiffuseParameter    = 0x0002,
    SpecularScale       = 0x0004,
    SpecularParameter   = 0x0008,
    Dependent           = 0x0010,
    Diffuse             = DiffuseScale | DiffuseParameter,
    Specular            = SpecularScale | SpecularParameter,
    Scale               = DiffuseScale | SpecularScale,
    Parameter           = DiffuseParameter | SpecularParameter,
    All                 = Specular | Diffuse,
  };

  BBM_ENUM(bsdf_attr, None, DiffuseScale, DiffuseParameter, SpecularScale, SpecularParameter, Dependent, Diffuse, Specular, Scale, Parameter, All);
  
} // end bbm namespace

#endif /* _BBM_BSDF_ATT_FLAG_H_ */
