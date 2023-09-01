#ifndef _BBM_DRJIT_STRINGCONVERT_H_
#define _BBM_DRJIT_STRINGCONVERT_H_

#include "util/typestring.h"
#include "core/error.h"
#include "concepts/stringconvert.h"

/************************************************************************/
/*! \file stringconvert.h

  \brief Specialization for converting backbone core types to and from strings

*************************************************************************/

namespace backbone {

  /**********************************************************************/
  /*! \brief regular drjit::array types
   **********************************************************************/
  template<typename T> requires (drjit::is_static_array_v<T> && !backbone::is_LLVMArray_v<T> && !backbone::is_DiffArray_v<T>)
    inline std::string toString(const T& arr)
  {
    // helper lambda to convert to std::array
    auto helper = [&]<size_t... IDX>(std::index_sequence<IDX...>) { return std::array{arr[IDX]...}; };

    // leverage bbm::toString for std::array
    return bbm::toString( helper(std::make_index_sequence<std::decay_t<T>::Size>{}) );
  }

  template<typename T> requires (drjit::is_static_array_v<T> && !backbone::is_LLVMArray_v<T> && !backbone::is_DiffArray_v<T>)
    inline T fromString(const std::string& str)
  {
    // convert str to std::array
    auto arr = bbm::fromString<std::array<value_t<T>, std::decay_t<T>::Size>>(str);

    // helper lambda to create T from std::array
    auto helper = [&]<size_t ...IDX>(std::index_sequence<IDX...>) { return T(arr[IDX]...); };

    // Done.
    return helper(std::make_index_sequence<std::decay_t<T>::Size>{});
  }

  /**********************************************************************/
  /*! \brief Handle drjit::DiffArray
   **********************************************************************/
  template<typename T> requires backbone::is_DiffArray_v<T>
    inline std::string toString(const T& val)
  {
    return bbm::toString( drjit::detach(val) );
  }

  template<typename T> requires backbone::is_DiffArray_v<T>
    inline T fromString(const std::string& str)
  {
    return T( bbm::fromString<drjit::value_t<T>>(str) );
  }
    
  /**********************************************************************/
  /*! \brief Handle drjit::LLVMArray
   **********************************************************************/
  template<typename T> requires backbone::is_LLVMArray_v<T>
    inline std::string toString(const T& val)
  {
    if(val.size() == 0) throw bbm_unassigned_ref;
    return bbm::toString( val[0] );
  }

  template<typename T> requires backbone::is_LLVMArray_v<T>
    inline T fromString(const std::string& str)
  {
    return T( bbm::fromString<drjit::value_t<T>>(str) );
  }
    
} // end backbone namespace

#endif /* _BBM_DRJIT_STRINGCONVERT_H_ */
