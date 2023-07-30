#ifndef _BBM_ENUMERATE_H_
#define _BBM_ENUMERATE_H_

#include <concepts>
#include "concepts/util.h"

/***********************************************************************/
/*! \file enumerate.h
  \brief Enumerate all elements in iterators recursively.

  Enumerate all elements of in an iterable type recursively
  until the element can be cast to a given VALUE type. The value is than 
  passed into a provided callback function:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  enumerate<float>(data, [&](auto& val) { some_vector.push_back(val); });
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  will enumerate all elements in data, and push them onto 'some_vector'.

************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief Convertible to VALUE => callback
   *********************************************************************/
  template<typename VALUE, typename T, typename CALLBACK> requires (std::convertible_to<T&, VALUE> && std::invocable<CALLBACK, VALUE>)
    void enumerate(T&& value, CALLBACK&& callback)
  {
    callback(std::forward<decltype(value)>(value));
  }
    
  /*********************************************************************/
  /*! \brief iterable object => enumerate(begin(), ..., end())
   *********************************************************************/
  template<typename VALUE, typename T, typename CALLBACK> requires (std::ranges::range<T> && !std::convertible_to<T&, VALUE> && std::invocable<CALLBACK, VALUE>)
    void enumerate(T&& obj, CALLBACK&& callback)
  {
    for(auto&& val : obj)
      enumerate<VALUE>(std::forward<decltype(val)>(val), std::forward<CALLBACK>(callback));
  }
    
  
  /*** Implementation details for is_enumerable ***/
  namespace detail {

    // default case
    template<typename VALUE, typename T> struct is_enumerable_impl : std::false_type {};

    // base case
    template<typename VALUE, typename T> requires std::is_convertible_v<T&, VALUE>
      struct is_enumerable_impl<VALUE, T> : std::true_type {};

    // iterable
    template<typename VALUE, typename T> requires (std::ranges::range<T> && !std::is_convertible_v<T&, VALUE>)
      struct is_enumerable_impl<VALUE, T> : is_enumerable_impl<VALUE, decltype( *std::begin(std::declval<T>()) ) > {};
    
  } // end detail namespace
  
  /*********************************************************************/
  /*! @{ \name is_enumerable
   *********************************************************************/
  template<typename VALUE, typename T>
    using is_enumerable = bbm::detail::is_enumerable_impl<VALUE, T>;

  template<typename VALUE, typename T>
    inline constexpr bool is_enumerable_v = is_enumerable<VALUE,T>::value;
  //! @}
  
} // end bbm namespace


#endif /* _BBM_ENUMERATE_H_ */
