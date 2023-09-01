#ifndef _BBM_ATTRIBUTE_VALUE_H_
#define _BBM_ATTRIBUTE_VALUE_H_

#include "concepts/attribute.h"

/************************************************************************/
/*! \file attribute_value.h

  \brief Helper methods for extracting the value of an attribute (according to
  concepts::attribute).

*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief return the value of an attribute, or if not an attribute the object
    *********************************************************************/
  template<typename T>
    decltype(auto) value(T&& t)
  {
    if constexpr (concepts::attribute<std::decay_t<T>>) return t.value();
    else return t;
  }

  
  /*** Implementation detail ***/
  namespace detail {
    template<typename T> struct attribute_value { using type = T; };
    template<typename T> requires bbm::concepts::attribute<T>
      struct attribute_value<T>
    {
      using type = typename std::decay_t<T>::type;
    };
  } // end detail namespace

  /**********************************************************************/
  /*! \brief return the type of value(t)
   **********************************************************************/
  template<typename T>
    using attribute_value_t = bbm::detail::attribute_value<std::decay_t<T>>::type;

} // end bbm namespace

#endif /* _BBM_ATTRIBUTE_VALUE_H_ */
