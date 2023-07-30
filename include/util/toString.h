#ifndef _BBM_TO_STRING_H_
#define _BBM_TO_STRING_H_

#include <sstream>
#include "util/type_traits.h"

/************************************************************************/
/*! \file toString.h
  \brief toString method for transforming types into a string.  A default
  sstream solution is provided, but overrides can be added if needed.
*************************************************************************/
#include "util/typestring.h"

namespace bbm {

  //! Make sure there is at least one bbm::operator<<
  namespace detail { struct empty {}; }
  std::ostream& operator<<(std::ostream& s, bbm::detail::empty) { return s; }
  
  /**********************************************************************/
  /*! \brief Concept to check if type has a toString member
   **********************************************************************/
  template<typename T>
    concept has_toString = requires(T&& t)
  {
    t.toString();
  };

  /**********************************************************************/
  /*! \brief Concept to check if type has a std::to_string
   **********************************************************************/
  template<typename T>
   concept has_to_string = requires
  {
    std::to_string(std::declval<T>());
  };
  
  /*********************************************************************/
  /*! \brief Convert a type to a string
   *********************************************************************/
  template<typename T>
    inline std::string toString(const T& t)
  {
    if constexpr (has_toString<T>) return t.toString();
    else if constexpr (is_string_type_v<T>) return t;
    else if constexpr (has_to_string<std::decay_t<T>>) return std::to_string(t);
    else
    {
      std::stringstream ss;
      ss << t;
      return ss.str();
    }
  }
} // end bbm namespace

#endif /* _BBM_TO_STRING_H_ */
