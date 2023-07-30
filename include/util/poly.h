#ifndef _BBM_POLY_H_
#define _BBM_POLY_H_

/************************************************************************/
/*! \file poly.h
  \brief Compile time polynomial using Horner's method.
*************************************************************************/

namespace bbm {

  /*** Implementation details ***/
  namespace detail {
    
    template<size_t MAXN, typename T, typename T0, typename... Ts>
      inline constexpr auto poly(T&& x, T0&& c0, Ts&&... c)
    {
      if constexpr (MAXN == 1 || sizeof...(Ts) == 0) return c0;
      else return poly<MAXN-1>(std::forward<T>(x), std::forward<Ts>(c)...)*x + c0;
    }
    
  } // end detail namespace

  
  /**********************************************************************/
  /*! \brief Compute a polynomial \f$ p(x) = \sum_{i=0} x^i * c_i \f$

    \param x = value to evaluate the polynomial at
    \param c0 = constant coefficient
    \param c... = remainder of coefficients.
    \returns the evaluation of the polynomial

    Determins the max degree based on the number of coefficients.
  ***********************************************************************/    
  template<typename T, typename T0, typename... Ts>
    inline constexpr auto poly(T&& x, T0&& c0, Ts&&... c)
  {
    return bbm::detail::poly<sizeof...(Ts)+1>(std::forward<T>(x), std::forward<T0>(c0), std::forward<Ts>(c)...);
  }

  
  /**********************************************************************/
  /*! \brief Compute a polynomial \f$ p(x) = \sum_{i=0}^N x^i * c_i \f$

    \tparam N = max polynomial degree
    \param x = value to evaluate the polynomial at
    \param c0 = constant coefficient
    \param c... = remainder of coefficients.
    \returns the evaluation of the polynomial

    Will only evaluate the coefficients less than max degree+1.
  ***********************************************************************/    
  template<size_t N, typename T, typename T0, typename... Ts>
    inline constexpr auto poly(T&& x, T0&& c0, Ts&&... c)
  {
    return bbm::detail::poly<N>(std::forward<T>(x), std::forward<T0>(c0), std::forward<Ts>(c)...);
  }
    
} // end bbm namespace

#endif /* _BBM_POLY_H_ */
