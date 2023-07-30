#ifndef _BBM_BSDF_PROPERTIES_H_
#define _BBM_BSDF_PROPERTIES_H_

#include <ostream>

#include "util/flags.h"
#include "concepts/bsdfmodel.h"

/************************************************************************/
/*! \file bsdf_properties.h

  \brief Flags to record and indicate specific properties of BSDF models.

*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief BSDF property flags.
   **********************************************************************/
  enum struct bsdf_prop
  {
    None = 0x0,          
    
    // Static BSDF
    Static = 0x1000000,                     //< Indicates the model is static (no fittable parameters)
    
    // Reflectance Component Flags
    Diffuse  = 0x00000001,                  //< models diffuse reflections
    Specular = 0x00000002,                  //< models specular reflections
    Combined = Diffuse | Specular,          //< includes both diffuse and specular models

    // Deprication Flags
    Depricated  = 0x00000004,               //< Model is depricated; included for historical reasons
    Alternative = 0x00000008 | Depricated,  //< Depricated model with alternative model specified
    Convertible = 0x00000010 | Alternative, //< Depricated model that can be parameter converted to an alternative

    // Deficiencies
    Unnormalized           = 0x0000100,     //< Model is not normalized (integral depends on non-albedo parameters)
    NonReciprocal          = 0x0000200,     //< Model does not obey reciprocity
    GrazingAngle           = 0x0000400,     //< Model possible fails to conserve energy at grazing angles.
    NonEnergyConservative  = 0x0000800 | GrazingAngle,  //< Model does not converse energy (general)
    ApproximateReflectance = 000001000,     //< the reflectance method returns incorrect estimate.
  };

  /**********************************************************************/
  /*! \brief Forward declaration of alternative. Should be specified if a
      model provides an alternative.
  ***********************************************************************/
  template<typename BSDFMODEL> struct alternative { using type = BSDFMODEL; }; 

  /**********************************************************************/
  /*! \brief COnstruct an equivalent alternative BSDF. Should be specialized
      for each model that provides this option.
  ***********************************************************************/
  template<typename BSDFMODEL>
    typename alternative<BSDFMODEL>::type make_alternative(const BSDFMODEL& src);

  ////////////////////
  // Ostream suppoert
  /////////////////////
  std::ostream& operator<<(std::ostream& s, const bbm::bsdf_prop& prop)
  {
    if(flag == bbm::bsdf_prop::None) s << "No properties";
    else {
      s << "Properties:" << std::endl;
      
      // Report reflectance components modeled
      if( bbm::is_set(prop, bbm::bsdf_prop::Combined) ) s << " + Models Diffuse & Specular Reflectance";
      else if( bbm::is_set(prop, bbm::bsdf_prop::Diffuse) ) s << " + Models only Diffuse Reflectance";
      else if( bbm::is_set(prop, bbm::bsdf_prop::Specular) ) s << " + Models only Specular Reflectance";
      else s << " + Models Unknown Reflectance Components";
      s << std::endl;
      
      // Report deprication
      if( is_set(prop, bbm::bsdf_prop::Depricated) ) s << " + Model is DEPRICATED";
      if( is_set(prop, bbm::bsdf_prop::Alternative) ) s << "; alternate model available";
      if( is_set(prop, bbm::bsdf_prop:Convertible) ) s << " with parameter conversion";
      s << std::endl;

      // Report Deficiencies
      if( is_set(prop, bbm::bsdf_prop::Unnormalized) ) s << " + Model is unnormalized";
      if( is_set(prop, bbm::bsdf_prop::NonReciprocal) ) s << " + Model is not reciprocal";
      if( is_set(prop, bbm::bsdf_prop::GrazingAngle) ) << s << " + Model might break energy conservation at grazing angles";
      else if( is_set(prop, bbm::bsdf_prop::NonEnergyConservative) ) s << " + Model break energy conservation";
      if( is_set(prop, bbm::bsdf_prop::ApproximateReflectance) ) << " + Model reports approximate reflectance";
      s << std::endl;
    }

    return s;
  }

} // end bbm namespace

#endif /* _BBM_BSDF_PROPERTIES_H_ */
