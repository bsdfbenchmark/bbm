#ifndef _BBM_CONFIG_CONCEPT_H_
#define _BBM_CONFIG_CONCEPT_H_

#include <concepts>
#include <type_traits>

#include "concepts/named.h"

/************************************************************************/
/*! \file config.h

  \brief config contract

*************************************************************************/

namespace bbm {
  namespace concepts {

  /**********************************************************************/
  /*! \brief config concept

    Each config struct contains:
    + concepts::named
    + a Config typedef that points to itself
    + a Value typedef
    + a Spectrum typedef
    + static Spectrum wavelength() that returns a Spectrum-type with the
      wavelength in each channel in nm.
   **********************************************************************/
  template<typename T>
    concept config = requires
  {
    requires concepts::named<T>;
    
    typename std::decay_t<T>::Config;
    typename std::decay_t<T>::Value;
    typename std::decay_t<T>::Spectrum;

    { std::decay_t<T>::wavelength() } -> std::same_as<typename std::decay_t<T>::Spectrum>;
    
    // Config must point to itself.
    requires std::same_as<typename std::decay_t<T>::Config, std::decay_t<T>>; 
  };


    
  /*********************************************************************/
  /*! \brief has_config

    Check if the object has a Config typedef.
  **********************************************************************/
  template<typename T>
    concept has_config = requires {typename std::decay_t<T>::Config;};

    
  /*********************************************************************/
  /*! \brief matching_config concept

    Checks if all types have the same config; will fail if any type is not
    a config.
  **********************************************************************/
  template<typename... U> 
    concept matching_config = (std::same_as<typename std::decay_t<std::tuple_element_t<0, std::tuple<U...>>>::Config, typename std::decay_t<U>::Config> && ...);


  /*********************************************************************/
  /*! \brief config archtype for concept cheking.
   *********************************************************************/
    namespace archetype {
      struct config {
        static constexpr string_literal name = "archetype";
        using Config = config;
        using Value = float;
        using Spectrum = bbm::color<float>;
        static constexpr Spectrum wavelength(void);
      };
    } // end arcetype namespace
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_CONFIG_CONCEPT_H_ */
