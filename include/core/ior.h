#ifndef _BBM_IOR_H_
#define _BBM_IOR_H_

#include <type_traits>

#include "core/attribute.h"

/***********************************************************************/
/*! \file ior.h
    \brief Defines 'ior' and 'reflectance' types

    Index of refraction (ior) and reflectance at normal incidence are two
    common methods for parameterizing Fresnel reflectance.  Both are floats.
    BBM defines both as an attribute variable with property 'tags':
    reflectance_tag and ior_tag.

    *************************************************************************/

namespace bbm {

  //! \brief Namespace for 'ior' and 'reflectance' types.
  namespace ior  {  

    //////////
    // Tags //
    //////////
    template<typename T> struct ior_tag { using type = T; };
    template<typename T> struct complex_ior_tag { using type = bbm::complex<T>; };
    template<typename T> struct reflectance_tag { using type = T; };
    
    ////////////////////////////////////
    //! @{ \name Type definition
    /////////////////////////////////////
    template<typename T> using ior = attribute<ior_tag<T>>;
    template<typename T> using complex_ior = attribute<complex_ior_tag<T>>;
    template<typename T> using reflectance = attribute<reflectance_tag<T>>;
    //! @}
    

    /*******************************************************************/
    /*! @{ \name type traits
     *******************************************************************/
    template<typename T> concept is_ior_v = concepts::has_attribute_property<T, ior_tag<typename std::decay_t<T>::type>>;
    template<typename T> concept is_complex_ior_v = concepts::has_attribute_property<T, complex_ior_tag<typename std::decay_t<T>::type>>;
    template<typename T> concept is_reflectance_v = concepts::has_attribute_property<T, reflectance_tag<typename std::decay_t<T>::type>>;

    template<typename T> concept is_ior_type = is_ior_v<T> || is_complex_ior_v<T> || is_reflectance_v<T>;
    //! @}

    
    /*******************************************************************/
    /*! @{ \name Conversion between 'ior' and 'reflectance' 
      \brief Conversion between 'ior' and 'reflectance' types

      \param target = type to copy to.
      \param source = which variable to copy from
    *******************************************************************/
    template<typename T>
      void convert(reflectance<T>& target, const ior<T>& i)
      {
        // ior to reflectance
        T temp = (T(i) - 1.0f) / (T(i) + 1.0f);
        target = temp*temp;
      }

    template<typename T>
      void convert(ior<T>& target, const reflectance<T>& r)
      {
        // reflectance to ior
        T temp = bbm::safe_sqrt(T(r));
        target = (1.0f + temp) / (1.0f - temp);
      }

    template<typename T>
      void convert(complex<T>& target, const ior<T>& i)
    {
      target = complex<T>(T(i), T(0));
    }
    //! @}
    
  } // end ior namespace

} // end bbm namespace

#endif /* _BBM_IOR_H_ */
