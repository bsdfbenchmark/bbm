#ifndef _BBM_ENOKI_STRINGCONVERT_H_
#define _BBM_ENOKI_STRINGCONVERT_H_

#include "concepts/stringconvert.h"

/************************************************************************/
/*! \file stringconvert.h

  \brief Specialization for converting backbone core types to and from strings

*************************************************************************/

namespace backbone {

  namespace detail {
    template<typename T> struct is_packet_mask : std::false_type {};
    template<typename T> struct is_packet_mask<enoki::PacketMask<T>> : std::true_type {};

    template<typename T>
      static constexpr bool is_packet_mask_v = is_packet_mask<std::decay_t<T>>::value;
  }
  
  /**********************************************************************/
  /*! \brief regular enoki::array types
   **********************************************************************/
  template<typename T> requires (enoki::is_static_array_v<T> && !backbone::is_DiffArray_v<T>)
    inline std::string toString(const T& arr)
  {
    using base_type = std::conditional_t<detail::is_packet_mask_v<T>, bool, value_t<T>>;
    
    // helper lambda to convert to std::array
    auto helper = [&]<size_t... IDX>(std::index_sequence<IDX...>) { return std::array<base_type, sizeof...(IDX)>{arr[IDX]...}; };

    // leverage bbm::toString for std::array
    return bbm::toString( helper(std::make_index_sequence<std::decay_t<T>::Size>{}) );
  }
  
  template<typename T> requires (enoki::is_static_array_v<T> && !backbone::is_DiffArray_v<T>)
    inline T fromString(const std::string& str)
  {
    using base_type = std::conditional_t<detail::is_packet_mask_v<T>, bool, value_t<T>>;
    
    // get compatible array
    auto arr = bbm::fromString<std::array<base_type, std::decay_t<T>::Size>>(str);

    // helper lambda for creating T from an std::array
    auto helper = [&]<size_t... IDX>(std::index_sequence<IDX...>)
    {
      // special case packet mask
      if constexpr (detail::is_packet_mask_v<T>)
      {
        T result;
        ((result[IDX] = arr[IDX]), ...);
        return result;
      }

      // otherwise (if not packtmask); direct construct
      else return T(arr[IDX]...);
    };
    
    // Done.
    return helper(std::make_index_sequence<std::decay_t<T>::Size>{});
  }

  /**********************************************************************/
  /*! \brief enoki::DiffArray 
   **********************************************************************/
  template<typename T> requires backbone::is_DiffArray_v<T>
    inline std::string toString(const T& val)
  {
    return bbm::toString( enoki::detach(val) );
  }

  template<typename T> requires backbone::is_DiffArray_v<T>
    inline T fromString(const std::string& str)
  {
    return T( bbm::fromString<enoki::value_t<T>>(str) );
  }

} // end backbone namespace
  
#endif /* _BBM_ENOKI_STRINGCONVERT_H_ */
