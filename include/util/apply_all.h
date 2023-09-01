#ifndef _BBM_APPLY_ALL_H_
#define _BBM_APPLY_ALL_H_

#include "concepts/util.h"
#include "util/attribute_value.h"
#include "util/type_traits.h"
#include "util/reflection.h"
#include "util/named.h"

namespace bbm {

  /*** Implementation details ***/
  namespace detail {
    template<typename T> struct is_gettable { static constexpr bool value = bbm::concepts::gettable<T>; };
    template<typename T> struct has_reflection { static constexpr bool value = bbm::concepts::reflection::supported<T>; };
    
    template<typename T> struct get_size { static constexpr size_t value = 1; };
    template<typename T> requires bbm::concepts::gettable<T> struct get_size<T> { static constexpr size_t value = std::tuple_size_v<std::decay_t<T>>; };
  }

  /**********************************************************************/
  /*! \brief Apply a function

    Applies a function to a series of arguments.  The following rules are
    used to determine the return type:

    + if any type support reflection AND all reflection types are the same AND
      reflection type supports empty construction, then call func on each
      reflection element.

    + if any type is gettable AND all gettable types have the same size, then
      apply func to all gettable elements. Try to keep the return type the
      same, or return a tuple.

    + else pass on to func directly
    *********************************************************************/
  template<typename FUNC, typename... T> requires (
        !std::is_void_v< find_first<bbm::detail::has_reflection, T...> > ||    // is reflection type OR
        !std::is_void_v< find_first<bbm::detail::is_gettable, T...> >    ||    // is gettable type OR
        requires(const FUNC& func, const T&... t) {{func(value(t)...)};}              // supports direct call
        )
    inline auto apply_all(const FUNC& func, const T&... t)
  {
    // helper functions
    auto get_if = []<size_t IDX>(auto&& v)   // get IDX-th element if v is gettable, otherwise return v
    {
      using V = decltype(v);
      if constexpr (concepts::gettable<V>) return std::get<IDX>(std::forward<V>(v));
      else return v;
    };

    auto reflected_if = [](auto&& v)         // return attributes if v supports reflection, otherwise return v
    {
      using V = decltype(v);
      if constexpr (concepts::reflection::supported<V>) return value_copy_tuple(reflection::attributes(v));
      else return v;
    };

    auto call_func = [&]<size_t IDX>()       // call 'func' on IDX-th parameters
    {
      return apply_all( func, get_if.template operator()<IDX>(t)... );
    };
      
    auto call_func_all = [&]<size_t... IDX>(std::index_sequence<IDX...>)  // enumerate 'func' over all IDXs' gettable parameters
    {
      return std::make_tuple( call_func.template operator()<IDX>()... );
    };

    // Determine reflection and gettable type
    using ReflType = find_first< bbm::detail::has_reflection, T...>;
    using GetType = find_first< bbm::detail::is_gettable, T...>;

    // CASE 1: reflection
    if constexpr ( !std::is_void_v<ReflType> && std::constructible_from<ReflType> && ((!concepts::reflection::supported<T> || std::same_as<reflection::attributes_t<ReflType>, reflection::attributes_t<T>>) && ...) )
    {
      ReflType result;
      reflection::attributes(result) = apply_all(func, reflected_if(t)...);
      return result;
    }

    // CASE 2: gettable
    else if constexpr ( !std::is_void_v<GetType> && ((bbm::detail::get_size<GetType>::value == bbm::detail::get_size<T>::value || !bbm::concepts::gettable<T>) && ...) )
    {
      auto result = call_func_all(std::make_index_sequence<bbm::detail::get_size<GetType>::value>{});
      
      // try to cast to GetType
      if constexpr (concepts::constructible_from_tuple<GetType, decltype(result)> && std::same_as<decltype(result), to_tuple_t<GetType>>)
       {
         return std::make_from_tuple<GetType>(result);
       }

      // otherwise return tuple
      else return result;
    }
    // ESLE: direct call
    else return func(value(t)...);
  }
      
} // end bbm namespace
  
#endif /* _BBM_APPLY_ALL_H_ */
