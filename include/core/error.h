#ifndef _BBM_ERROR_H_
#define _BBM_ERROR_H_

#include <stdexcept>
#include "util/macro_util.h"

/***********************************************************************/
/*! \file error.h
    \brief Predefined exceptions for common errors
************************************************************************/

namespace bbm {
  /*********************************************************************/
  /* \brief Helper class for throwing a static assert in the 'else' condition
     of a 'if constexpr' statement.  Directly using 'false' in the static_expression
     always yields a complile error, even if the else condition was not taken.
     A work around is to make the condition of the static_assert dependent
     on a template type.  Example:

     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
     template<typename T>
       T foo(T t)
     {
        if constexpr (std::same_as<T, float>) { return t; }
        else static_assert( dependent_false_v<T>, "T is not a float");
     }
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     
  ***********************************************************************/
  template<typename... T> struct dependent_false : public std::false_type {};

  template<typename... T>
    static constexpr bool dependent_false_v = dependent_false<T...>::value;

  template<typename... T>
    static constexpr bool dependent_true_v = !dependent_false<T...>::value;
} // end bbm namespace

#define BBM_ERROR_MSG(...) BBM_STRINGIFY( __VA_ARGS__ (__FILE__ : __LINE__). )

//! @{ \name Predefined Run-time Errors
#define bbm_incomplete_init std::runtime_error(BBM_ERROR_MSG(BBM: accessing incompletely initialized object))
#define bbm_unassigned_ref std::runtime_error(BBM_ERROR_MSG(BBM: Unassigned references cannot be cast/read))
#define bbm_out_of_range std::runtime_error(BBM_ERROR_MSG(BBM: index out of range))
#define bbm_size_error std::runtime_error(BBM_ERROR_MSG(BBM: size error))
#define bbm_unevaluable std::runtime_error(BBM_ERROR_MSG(BBM: cannot evaluate expression))
//! @}

//! @{ \name Predefined Compile-time Errors
#define BBM_SIZE_MISMATCH "BBM: size mismatch" 
#define BBM_NO_MATCH "BBM: no matching argument found"
#define BBM_NOT_DIFF "BBM: type does not support gradients"
#define BBM_INVALID_LAMBDA "BBM: invalid lambda signature"
#define BBM_CONST_ASSIGNMENT "BBM: cannot assign to a const type"
//! @}

#endif /* _BBM_ERROR_H_ */
