#ifndef _BBM_CONCEPTS_UTIL_H_
#define _BBM_CONCEPTS_UTIL_H_

#include <tuple>

/***********************************************************************/
/*! \file util.h

  \brief Additional basic helper concepts.
************************************************************************/

namespace bbm {
  namespace concepts {

  /**********************************************************************/
  /*! \brief Concept wrapper around a type trait: check if T meets the type TRAIT<T,ARGS...>
   **********************************************************************/
  template<typename T, template<typename...> typename TRAIT, typename... ARGS>
    concept trait_wrapper = TRAIT<T, ARGS...>::value;

  /*********************************************************************/
  /* \brief true if type trait exists for a type
   *********************************************************************/
  template<template<typename...> typename TRAIT, typename... ARGS>
    concept valid_trait = std::same_as<TRAIT<ARGS...>, TRAIT<ARGS...>>;

  /*********************************************************************/
  /*! \brief true if type is the same as any of the listed types
   *********************************************************************/
  template<typename T, typename... U>
    concept same_as_any = (std::same_as<T, U> || ...);

    /*********************************************************************/
  /*! \brief true if type is the same as all of the listed types
   *********************************************************************/
  template<typename T, typename... U>
    concept same_as_all = (std::same_as<T, U> && ...);

  /*********************************************************************/
  /*! \brief assignable_to: reverse of std::assignable_from
   *********************************************************************/
  template<typename RHS, typename LHS>
  concept assignable_to = requires(RHS&& r, LHS l)
  {
    { l = std::forward<RHS>(r) };
  };
    
  /*********************************************************************/
  /*! @{ \name basic math operator concepts 
   *********************************************************************/
    template<typename T>
    concept has_addition = requires(T a)
  {
    { a+a } -> assignable_to<T>;    // T+T
    { a+1 } -> assignable_to<T>;    // T+scalar
    { a+=a };   // T+=T
    { a+=1 };   // T+=scalar
  };

  template<typename T>
    concept has_subtraction = requires(T a)
  {
    { a-a } -> assignable_to<T>;    // T-T
    { a-1 } -> assignable_to<T>;    // T-scalar
    { a-=a };   // T-=T
    { a-=1 };   // T-=scalar    
  };

  template<typename T>
    concept has_multiplication = requires(T a)
  {
    { a*a } -> assignable_to<T>;    // T*T
    { a*1 } -> assignable_to<T>;    // T*scalar
    { a*=a };   // T*=T
    { a*=1 };   // T*=scalar
  };

  template<typename T>
    concept has_division = requires(T a)
  {
    { a/a } -> assignable_to<T>;    // T/T
    { a/1 } -> assignable_to<T>;    // T/scalar
    { a/=a };   // T/=T
    { a/=1 };   // T/=scalar
  };

  template<typename T>
    concept has_modulo = requires(T a)
  {
    { a%a } -> assignable_to<T>;    // T%T
    { a%1 } -> assignable_to<T>;    // T%scalar
    { a%=a };   // T%=T
    { a%=1 };   // T%=scalar
  };

  template<typename T>
    concept has_basic_math = has_addition<T> && has_subtraction<T> && has_multiplication<T> && has_division<T>;
  //! @}

  
  /**********************************************************************/
  /*! @{ \name increment/decrement operators
   **********************************************************************/
  template<typename T>
    concept has_increment = requires(T a)
  {
    { a++ };
    { ++a };
  };

  template<typename T>
    concept has_decrement = requires(T a)
  {
    { a-- };
    { --a };
  };
  //! @}

    
  /**********************************************************************/
  /*! \brief bit operators
   **********************************************************************/
  template<typename T>
    concept has_bit_operators = requires(T a)
  {
    { a | a } -> assignable_to<T>;
    { a & a } -> assignable_to<T>;
    { a ^ a } -> assignable_to<T>;
    { ~a } -> assignable_to<T>;
    { a |= a };
    { a &= a };
    { a ^= a };
  };

  /**********************************************************************/
  /*! \brief logical operators
   **********************************************************************/
  template<typename T>
    concept has_logical_operators = requires(T a)
  {
    { a || a } -> assignable_to<T>;
    { a && a } -> assignable_to<T>;
    { !a } -> assignable_to<T>;
  };
  
  /**********************************************************************/
  /*! @{ \name dereference/arrow operations
   **********************************************************************/
  template<typename T>
    concept has_dereference = requires(T a)
  {
    { *a };
  };

  template<typename T>
    concept has_arrow = requires(T a)
  {
    { a.operator->() };
  };
  //! @}
    
  /**********************************************************************/
  /*! \brief constructible_from_tuple concept
   **********************************************************************/
  template<typename T, typename TUP>
    concept constructible_from_tuple = requires
  {
    { std::make_from_tuple<T>( std::declval<TUP>() ) };
  };

  /**********************************************************************/
  /*! \brief std::get supported
   **********************************************************************/
  template<typename T>
    concept gettable = requires(T a)
  {
    { std::get<0>(a) };
    { std::tuple_size<std::decay_t<T>>::value };
  };

  /**********************************************************************/
  /*! \brief Check if lambda/functor takes a set of arguments.
   **********************************************************************/
  template<typename L, typename... ARGS>
    concept lambda = requires(L lambda, ARGS... args)
  {
    { lambda(args...) };
  };

  } // end concepts namespace
} // end bbm namespace


#endif /* _BBM_CONCEPTS_UTIL_H_ */
