#ifndef _BBM_MAKE_FROM_H_
#define _BBM_MAKE_FROM_H_

#include "core/error.h"

#include "util/type_traits.h"
#include "util/literal.h"



/************************************************************************/
/*! \file make_from.h

  \brief Tools for making an object:

  1) from a series of literals
  2) from an array literal
  3) from a parameter pack
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Make an object T from a series of non-array template literals

    \tparam T = object-type to create
    \tparam V = template literals to pass as constructor arguments.
    \returns T(V...)

    Will be at compile time if the constructor of T is constexpr.
  ***********************************************************************/
  template<typename T, literal... V> requires std::constructible_from<T, decltype(V.value)...> && (!(is_array_v<decltype(V.value)> && ...))
    inline constexpr T make_from() { return T(V.value...); }

  
  /**********************************************************************/
  /*! \brief Make an object T from the values passed in a array literal

    \tparam T = object-type to create
    \tparam A = std::array<arg_type, N>, i.e., N arguments to be passed to T.
    \returns T(A[0], A[1], ...., A[N])
  ***********************************************************************/
  template<typename T, literal A> requires bbm::is_array_v<decltype(A.value)>
    inline constexpr T make_from()
  {
    auto helper = []<size_t... IDX>(std::index_sequence<IDX...>) { return T{A.value[IDX]...}; };
    return helper(std::make_index_sequence<std::tuple_size_v<decltype(A.value)>>{});
  }

  /**********************************************************************/
  /*! \brief Make an object T from a pack of arguments.

    \tparam T = object type to construct
    \param args = argument pack
    \returns T(args...)

    This function is essentially identical to directly calling T(args...)
    except for one key difference: this method will compile even if T is not
    constructible from 'args'.  At run-time an exeption is thrown in the
    latter case.
  ***********************************************************************/
  template<typename T, typename... Args>
    inline constexpr T make_from(Args&&... args)
  {
    if constexpr (std::constructible_from<T, Args...>)
    {
      if constexpr (sizeof...(Args) == 0) return T{};
      else return T(std::forward<Args>(args)...);
    }
    else throw bbm_unevaluable;
  }

  
} // end bbm namespace

#endif /* _BBM_MAKE_FROM_H_ */
