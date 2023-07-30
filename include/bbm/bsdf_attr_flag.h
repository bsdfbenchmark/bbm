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

  /////////////////////
  // ostream support //
  /////////////////////
  std::ostream& operator<<(std::ostream& s, const bbm::bsdf_attr& flag)
  {
    if(is_set(flag, bbm::bsdf_attr::Dependent))
    {
      s << "Depdent";
      if( !is_set(flag & ~bbm::bsdf_attr::Dependent, bbm::bsdf_attr::None) ) s << " ";
    }
    
    if(flag == bbm::bsdf_attr::None) s << "None";
    else if(flag == bbm::bsdf_attr::All) s << "All";
    else
    {
      std::vector<std::string> prop;
      if( bbm::is_set(bbm::bsdf_attr::Diffuse, flag)) prop.push_back("Diffuse");
      if( bbm::is_set(bbm::bsdf_attr::Specular, flag)) prop.push_back("Specular");
      if( bbm::is_set(bbm::bsdf_attr::Scale, flag) && !is_set(bbm::bsdf_attr::Parameter, flag)) prop.push_back("Scale");
      if( bbm::is_set(bbm::bsdf_attr::Parameter, flag) && !is_set(bbm::bsdf_attr::Scale, flag)) prop.push_back("Parameter");

      for(auto& p : prop)
      {
        s << p;
        if(&p != &prop.back()) s << " ";
      }
    }

    return s;
  }
} // end bbm namespace

#endif /* _BBM_BSDF_ATT_FLAG_H_ */
