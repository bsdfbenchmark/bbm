#ifndef _BBM_BSDF_ATTRIBUTE_CONCEPT_H_
#define _BBM_BSDF_ATTRIBUTE_CONCEPT_H_

#include "concepts/attribute.h"
#include "bbm/bsdf_attr_flag.h"

/************************************************************************/
/*! \file bsdf_attribute.h

    \brief bsdf attribute contract
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief bsdf_attribute_property concept
    
      Each bsdf attribute requires:
      - to be a attribute_property
      - contain a bsdf_attr flag
      - cotnain a default value static function
      - contain a lower bound static function
      - contain an upper bound static function
    *********************************************************************/
    template<typename PROP>
      concept bsdf_attribute_property = requires
    {
      requires concepts::attribute_property<PROP>;
      PROP::default_value();
      PROP::upper_bound();
      PROP::lower_bound();
      PROP::flag;
      requires std::same_as<std::decay_t<decltype(PROP::flag)>, bsdf_attr>;
    };

    /********************************************************************/
    /*! \brief bsdf_attribute

      Meets:
      + concepts::attribute<T>
      + concepts::bsdf_attribute_property<T::type>
    *********************************************************************/
    template<typename T>
      concept bsdf_attribute = concepts::attribute<T> && concepts::bsdf_attribute_property<typename std::decay_t<T>::prop>;
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_BSDF_ATTRIBUTE_CONCEPT_H_ */
