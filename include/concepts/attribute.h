#ifndef _BBM_ATTRIBUTE_CONCEPT_H_
#define _BBM_ATTRIBUTE_CONCEPT_H_

#include <utility>

/************************************************************************/
/*! \file attribute.h

  \brief attribute and attribute_property contracts
*************************************************************************/

namespace bbm {
  namespace concepts {

    /******************************************************************/
    /*! \brief attribute_property concept
   
      Each attribute_property must at least contain a typedef named 'type'
      that indicates the underlying attribute type.
    *******************************************************************/
    template<typename PROP>
      concept attribute_property = requires
    {
      typename std::decay_t<PROP>::type;
    };

    
    /********************************************************************/
    /*! \brief attribute concept

      Each attribute provides the following:
      + typename type : the underlying type
      + typename prop : requires attribute_property
      + convertible_to the underlying (reference) type
      + auto value() : return the cast to a reference of the underlying type
      + auto value() const : return the cast to a const reference of the underlying type
    *********************************************************************/
    template<typename T>
    concept attribute = requires(std::decay_t<T>& attr)
    {
      //! \brief has a type typename
      typename std::decay_t<T>::type;

      //! \brief has prop typename that also meet concepts::attribute_property
      requires concepts::attribute_property<typename std::decay_t<T>::prop>;

      //! @{ \name value methods
      { attr.value() } -> std::same_as<typename std::decay_t<T>::type&>;
      { std::as_const(attr).value() } -> std::same_as<const typename std::decay_t<T>::type&>;
      //! @}

      //! @{ \name cast
      static_cast<typename std::decay_t<T>::type&>(attr);
      static_cast<const typename std::decay_t<T>::type&>(attr);
      //! @}
    };

    
    /********************************************************************/
    /*! \brief has_attribute_property concept

      Checks if a type is an attribute with a given property
    *********************************************************************/
    template<typename T, typename PROP>
      concept has_attribute_property = attribute<T> && attribute_property<PROP> && std::same_as<PROP, typename std::decay_t<T>::prop>;
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_ATTRIBUTE_CONCEPT_H_ */
