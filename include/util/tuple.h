#ifndef _BBM_TUPLE_H_
#define _BBM_TUPLE_H_

#include <tuple>
#include <ostream>

#include "util/constfor.h"
#include "util/type_traits.h"

/************************************************************************/
/*! \file tuple.h
  \brief Extensions for the STL tuple class
*************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief Create a tuple from any other type that supports std::get
   *********************************************************************/
  template<typename T> requires concepts::gettable<T>
  inline auto to_tuple(T&& t)
  {
    auto convert = [&]<size_t... IDX>(std::index_sequence<IDX...>)
    {
      return std::make_tuple( std::get<IDX>(std::forward<T>(t))... );
    };

    return convert(std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>{});
  }

  /**********************************************************************/
  /*! \brief type of converting a type that supports std::get to a tuple
   **********************************************************************/
  template<typename T> requires concepts::gettable<T>
    using to_tuple_t = decltype( to_tuple(std::declval<T>()) ); 
  
  
 /*********************************************************************/
  /*! \brief Make a tuple of references.

    Returns std::tuple<ARGS...>, and therefore this method differs from:
    + std::make_tuple which returns a std::tuple<std::decay_t<ARGS>...>
    + std::forward_as_tuple which returns a std::tuple<ARGS&&...>
   *********************************************************************/
  template<typename... ARGS>
    inline constexpr auto make_ref_tuple(ARGS&&... args)
  {
    return std::tuple< ARGS... >( std::forward<ARGS>(args)... );
  }

  /**********************************************************************/
  /*! \brief Value-copy a tuple. 

    \param tup = tuple, possibly with references
    \returns a copy of the tuple without references.

    For example std::tuple<int&, int> will be converted to std::tuple<int, int>

  ***********************************************************************/
  template<typename... ARGS>
    inline constexpr auto value_copy_tuple(const std::tuple<ARGS...>& tup)
  {
    auto deref = [&]<size_t... IDX>(std::index_sequence<IDX...>)
    {
      return std::make_tuple( std::remove_cvref_t<ARGS>( std::get<IDX>(tup) )... );
    };

    return deref(std::make_index_sequence<sizeof...(ARGS)>{});
  }

  //! \brief value-copy type of a tuple.
  template<typename T> requires is_tuple_v<T>
    using value_copy_tuple_t = decltype( value_copy_tuple( std::declval<std::decay_t<T>>() ) );

  /**********************************************************************/
  /*! \brief tuple_cat_t

    Returns the type of std::tuple_cat
  ***********************************************************************/
  template<typename... Ts>
    using tuple_cat_t = decltype( std::tuple_cat( std::declval<Ts>()... ) );


  /**********************************************************************/
  /*! \brief subtuple

    \tparam START = start index of elements to include in the new tuple
    \tparam COUNT  = number of elements to include
  ***********************************************************************/
  template<size_t START, size_t COUNT, typename TUP> requires is_tuple_v<TUP> && ((START+COUNT) <= std::tuple_size_v<std::decay_t<TUP>>)
    inline constexpr auto subtuple(TUP&& tup)
  {
    // if zero length => return empty
    if constexpr (COUNT == 0) return std::tuple<>{};

    // select subset
    else
    {
      // helper lambda to extract the elements
      auto extract = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        return std::tuple<std::tuple_element_t<START+IDX, std::decay_t<TUP>>...>(std::get<START+IDX>(std::forward<TUP>(tup))...);
      };

      return extract(std::make_index_sequence<COUNT>{});
    }
  }

  /**********************************************************************/
  /*! \brief subtuple type
   **********************************************************************/
  template<size_t START, size_t COUNT, typename TUP> requires is_tuple_v<TUP> && ((START+COUNT) <= std::tuple_size_v<std::decay_t<TUP>>)
    using subtuple_t = decltype( subtuple<START,COUNT>(std::declval<TUP>()) );
  
  
  /**********************************************************************/
  /*! \brief Recursively flatten a tuple

    The resulting tuple is the concatenation of all elements of child tuples.
  ***********************************************************************/
  template<typename T>
    inline constexpr auto tuple_flatten(T&& t)
  {
    // base case: not a tuple
    if constexpr (!is_tuple_v<std::decay_t<T>>) { return std::forward_as_tuple(t); }

    // recursion: cat elements
    else
    {
      auto flatten = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        return std::tuple_cat( (tuple_flatten( std::get<IDX>(std::forward<T>(t)) ))... );
      };

      return flatten(std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>{});
    }

    // Done.
  }

  //! \brief flattened tuple type
  template<typename T>
    using tuple_flatten_t = decltype( tuple_flatten( std::declval<std::decay_t<T>>() ) );

  
  /**********************************************************************/
  /*! \brief tuple_add_const to each element
   **********************************************************************/
  template<typename T>
    inline constexpr auto tuple_add_const(T&& t)
  {
    auto add_const = [&]<size_t... IDX>(std::index_sequence<IDX...>)
    {
      return std::tuple< bbm::add_const_t<std::tuple_element_t<IDX, std::decay_t<T>>>... >( std::get<IDX>(std::forward<T>(t))... );
    };

    return add_const(std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>{});
  }

  //! \brief tuple_add_const type
  template<typename T>
    using tuple_add_const_t = decltype( tuple_add_const( std::declval<std::decay_t<T>>() ) );
  
  /**********************************************************************/
  /*! \brief tuple_remove_const from each element
   **********************************************************************/
  template<typename T>
    inline constexpr auto tuple_remove_const(T&& t)
  {
    auto cast = []<typename V>(V&& val) -> decltype(auto)
    {
      if constexpr (std::is_pointer_v<V> || std::is_reference_v<V>) return const_cast<bbm::remove_const_t<V>>(std::forward<V>(val));
      else return std::forward<V>(val);
    };
    
    auto remove_const = [&]<size_t... IDX>(std::index_sequence<IDX...>)
    {
      return std::tuple< bbm::remove_const_t<std::tuple_element_t<IDX, std::decay_t<T>>>... >( cast(std::get<IDX>(std::forward<T>(t)))... );
    };

    return remove_const(std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>{});
  }

  //! \brief tuple_remove_const type
  template<typename T>
    using tuple_remove_const_t = decltype( tuple_remove_const( std::declval<std::decay_t<T>>() ) );


} // end bbm namespace


namespace std {
  /**********************************************************************/
  /*! \brief print a tuple

    The tuple elements are surrounded by '(' and ')'.  Each tuple element is
    converted to a string with bbm::toString. String types are additionally
    surrounded by ""
  ***********************************************************************/
  template<typename... Ts>
    std::ostream& operator<<(std::ostream& s, const std::tuple<Ts...>& tup)
  {
    s << "(";
    CONSTFOR(idx, sizeof...(Ts),
    {
      if constexpr (idx != 0) s << ", ";

      // add "" if a string type
      if constexpr (bbm::is_string_type_v< std::tuple_element_t<idx, std::tuple<Ts...>> >) s << std::string("\"") + std::get<idx>(tup) + std::string("\"");

      // otherwise, just print value
      else s << std::get<idx>(tup);
    });
    s << ")";

    return s;
  }
  
} // end std namespace

#endif /* _BBM_TUPLE_H_ */
