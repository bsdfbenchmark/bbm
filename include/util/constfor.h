#ifndef _BBM_CONSTFOR_H_
#define _BBM_CONSTFOR_H_

/***********************************************************************/
/*! \file constfor.h
    \brief Complile-time for loop

    Expects the body to be passed as a templated lambda that takes a size_t
    literal as template parameter, and no arguments.

    Example:
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
    constfor<4>( []<size_t IDX>() 
    {
      std::cerr << IDX << std::endl;
    });
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Prints values from 0 to 3.

    A helper macro simplifies the call:
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
    CONSTFOR(IDX, 4,
    {
       std::cerr << IDX << std::endl;
    });
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*************************************************************************/

namespace bbm {

  /********************************************************************/
  /*! \brief Concept to check if a lambda function meets the required
    signature
  *********************************************************************/
  template<typename F>
  concept has_constfor_lambda = requires(F&& f) {
    f.template operator()<size_t(0)>();
  };
  
  /*********************************************************************/
  /*! \brief constfor given an index sequence of indexes.
   *********************************************************************/
  template<typename F, size_t... IDX> requires (sizeof...(IDX) == 0 || has_constfor_lambda<F>)
    inline constexpr void constfor(F&& f, std::index_sequence<IDX...>) 
  {
    (f.template operator()<IDX>(), ...);
  }

  /*********************************************************************/
  /*! \brief constfor given the number of iterations
   *********************************************************************/
  template<size_t NumItr, typename F> requires (NumItr == 0 || has_constfor_lambda<F>)
    inline constexpr void constfor(F&& f)
  {
    constfor(std::forward<F>(f), std::make_index_sequence<NumItr>{});
  }

  
  /*********************************************************************/
  /*! \brief HELPER MACRO
   *********************************************************************/
#define CONSTFOR(IDX, NUMITR, ...) bbm::constfor<NUMITR>( [&]<size_t IDX>() { __VA_ARGS__; } );
  
} // end bbm namespace


#endif /* _BBM_CONSTFOR_H_ */
