#ifndef _BBM_CONSTFOREACH_H_
#define _BBM_CONSTFOREACH_H_

/************************************************************************/
/*! \file constforeach.h
  \brief Compile-time for each loop

  Expects the body to passed as a templated lambda that takes the typenames
  and no arguments.

  Example:
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  constforeach<Ts...>( [&]<typename T>()
  {
    std::cerr << typestring<T> << std::endl;
  });
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Prints the typestrings of all typenames in Ts.

  A helper macro simplifies the call:
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  CONSTFOREACH(T, TS,
  {
    std::cerr << typestring<T> << std::endl;
  });
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Concept to check if a lambda functions meets the required signature
   **********************************************************************/
  template<typename F, typename... T>
    concept has_constforeach_lambda = requires(F&& f)
  {
    (f.template operator()<T>(), ...);
  };

  /**********************************************************************/
  /*! \brief constforeach over all typenames in T.
   **********************************************************************/
  template<typename... T, typename F> requires has_constforeach_lambda<F, T...>
    inline constexpr void constforeach(F&& f)
  {
    (f.template operator()<T>(), ...);
  }

  /**********************************************************************/
  /*! \brief HELPER MACRO
   **********************************************************************/
  #define CONSTFOREACH(ITR_TYPE, TYPE_LIST, ...) if constexpr (sizeof...(TYPE_LIST) > 0) bbm::constforeach<TYPE_LIST...>( [&]<typename ITR_TYPE>() { __VA_ARGS__ } );

} // end bbm namespace


#endif /* _BBM_CONSTFOREACH_H_ */
