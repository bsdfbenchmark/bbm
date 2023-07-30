#ifndef _BBM_BSDF_ATTRIBUTE_H_
#define _BBM_BSDF_ATTRIBUTE_H_

#include <limits>

#include "concepts/bsdf_attribute.h"

#include "bbm/bsdf_attr_flag.h"
#include "util/make_from.h"
#include "core/attribute.h"
#include "core/ior.h"



/***********************************************************************/
/*! \file bsdf_attribute.h
  \brief Default bsdf attribute types

  List of types:
  + bsdf_attribute<type, Flag, Default=1.0, Upperbound=max, Lowerbound=0.0>
  + bsdf_scale<type, Flag>
  + diffuse_scale<type>
  + specular_scale<type>
  + bsdf_roughness<type, Flag>
  + diffuse_roughness<type>
  + specular_roughness<type>
  + bsdf_sharpness<type, Flag>
  + diffuse_sharpness<type>
  + specular_sharpness<type>
  + fresnel_parameter<type, Flag==bsdf_attr::SpecularParameter>
************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! @{ \name Helper Methods for extracting default value / bounds
   *********************************************************************/
  template<typename T> requires concepts::bsdf_attribute<T>
    inline constexpr auto default_value(T) { return std::decay_t<T>::prop::default_value(); }

  template<typename T> requires concepts::bsdf_attribute<T>
    inline constexpr auto lower_bound(T) { return std::decay_t<T>::prop::lower_bound(); }

  template<typename T> requires concepts::bsdf_attribute<T>
    inline constexpr auto upper_bound(T) { return std::decay_t<T>::prop::upper_bound(); }

  template<typename T> requires concepts::bsdf_attribute<T>
  inline constexpr auto bsdf_attr_flag(T) { return std::decay_t<T>::prop::flag; }
  
  /*********************************************************************/
  /*! \brief bsdf_parameter property

    \tparam T = type of the value
    \tparam Flag = bsdf_attr flag
    \tparam Default = default value (default = 1)
    \tparam UpperBound = upper bound value (default = std::numeric_limits<scalar_t<type>>::max())
    \tparam LowerBound = lower bound (default = 0)
   *********************************************************************/
  template<typename T, bsdf_attr Flag, literal Default=1.0, literal UpperBound = std::numeric_limits<scalar_t<T>>::max(), literal LowerBound=0.0>
    struct bsdf_properties
  {
    using type = T;
    static constexpr bsdf_attr flag = Flag;
    inline static type default_value(void) { return make_from<T,Default>(); }
    inline static type lower_bound(void)   { return make_from<T,LowerBound>(); }
    inline static type upper_bound(void)   { return make_from<T,UpperBound>(); }
  };

  ////////////////////////////////////
  //! @{ \name Predefined Default BSDF attributes  
  ////////////////////////////////////
  template<typename T, bsdf_attr Flag, literal Default=1.0, literal UpperBound = std::numeric_limits<scalar_t<T>>::max(), literal LowerBound=0.0>
    using bsdf_parameter = attribute<bsdf_properties<T, Flag, Default, UpperBound, LowerBound>>;
  
  template<typename T, bsdf_attr Flag> using bsdf_scale = attribute<bsdf_properties<T, Flag, 0.5, 1.0>>;
  template<typename T> using diffuse_scale  = bsdf_scale<T, bsdf_attr::DiffuseScale>;
  template<typename T> using specular_scale = bsdf_scale<T, bsdf_attr::SpecularScale>;

  // roughness
  template<typename T, bsdf_attr Flag> using bsdf_roughness = attribute<bsdf_properties<T, Flag, 0.1, 1.0, constants<scalar_t<T>>::Epsilon()>>;
  template<typename T> using diffuse_roughness  = bsdf_roughness<T, bsdf_attr::DiffuseParameter>;
  template<typename T> using specular_roughness = bsdf_roughness<T, bsdf_attr::SpecularParameter>;

  // sharpness = ~1/roughness
  template<typename T, bsdf_attr Flag> using bsdf_sharpness = attribute<bsdf_properties<T, Flag, 32.0>>; 
  template<typename T> using diffuse_sharpness  = bsdf_sharpness<T, bsdf_attr::DiffuseParameter>;
  template<typename T> using specular_sharpness = bsdf_sharpness<T, bsdf_attr::SpecularParameter>;

  // ior and reflectance (R0)
  template<typename T, bsdf_attr Flag = bsdf_attr::SpecularParameter> struct bsdf_fresnel_properties;
  template<typename T, bsdf_attr Flag> struct bsdf_fresnel_properties<ior::ior<T>, Flag> : bsdf_properties<ior::ior<T>, Flag, 1.3, 5.0, 1.0> {};
  template<typename T, bsdf_attr Flag> struct bsdf_fresnel_properties<ior::reflectance<T>, Flag> : bsdf_properties<ior::reflectance<T>, Flag, 0.1, 1.0> {};
  template<typename T, bsdf_attr Flag> struct bsdf_fresnel_properties<ior::complex_ior<T>, Flag> : bsdf_properties<ior::complex_ior<T>, Flag, std::array{1.3, 0.0}, std::array{5.0, 10.0}, std::array{0.1, 0.0}> {};

  template<typename T, bsdf_attr Flag = bsdf_attr::SpecularParameter>
    using fresnel_parameter = attribute<bsdf_fresnel_properties<T,Flag>>;
  //! @}

}
  
#endif /* _BBM_BSDF_ATTRIBUTE_H_ */
