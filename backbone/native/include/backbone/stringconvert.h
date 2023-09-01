#ifndef _BBM_NATIVE_STRINGCONVERT_H_
#define _BBM_NATIVE_STRINGCONVERT_H_

#include <string>
#include "concepts/stringconvert.h"

/************************************************************************/
/*! \file stringconvert.h

  \brief Specializations for converting backbone core types to and from strings

*************************************************************************/

namespace backbone {

  template<typename T> requires backbone::is_array_v<T>
    inline std::string toString(const T& arr)
  {
    return bbm::toString<std::array<backbone::value_t<T>, backbone::array_size<T>>>(arr);
  }

  template<typename T> requires backbone::is_array_v<T>
    inline T fromString(const std::string& str)
  {
    return backbone::array<backbone::value_t<T>, backbone::array_size<T>>(bbm::fromString<std::array<backbone::value_t<T>, backbone::array_size<T>>>(str));
  }

} // end backbone namespace

#endif /* _BBM_NATIVE_STRIGCONVERT_H_ */
