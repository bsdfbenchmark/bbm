#ifndef _BBM_DRJIT_HORIZONTAL_H_
#define _BBM_DRJIT_HORIZONTAL_H_

/************************************************************************/
/*! \file horizontal.h

  \brief Map bbm horizontal methods to corresponding DrJIT methods

*************************************************************************/

namespace backbone {

  /**********************************************************************/
  /*! @{ Direct mapping to DrJIT methods if not a scalar
    *********************************************************************/
  template<typename T>
    inline auto dot(const T& a, const T& b)
  {
    if constexpr (std::is_scalar_v<T>) return a*b;
    else return drjit::dot(a, b);
  }

  template<typename T>
    inline auto norm(const T& a)
  {
    if constexpr (std::is_scalar_v<T>) return drjit::abs(a);
    else return drjit::norm(a);
  }
  
  template<typename T>
    inline auto squared_norm(const T& a)
  {
    if constexpr (std::is_scalar_v<T>) return a*a;
    else return drjit::squared_norm(a);
  }
  
  template<typename T>
  inline auto normalize(const T& a)
  {
    if constexpr (std::is_scalar_v<T>) return T(1);
    else return drjit::normalize(a);
  }
  
  using drjit::all;
  using drjit::any;
  using drjit::none;
  using drjit::count;
  //! @}

  /**********************************************************************/
  /*! @{ \name Passthough to DrJIT (name change)
   **********************************************************************/
  template<typename T> inline auto hsum(const T& t) { return drjit::sum(t); }
  template<typename T> inline auto hprod(const T& t) { return drjit::prod(t); }
  template<typename T> inline auto hmax(const T& t) { return drjit::max(t); }
  template<typename T> inline auto hmin(const T& t) { return drjit::min(t); }
  //! @}
    
} // end backbone namespace

#endif /* _BBM_DRJIT_HORIZONTAL_H_ */
