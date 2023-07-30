#ifndef _BBM_DRJIT_COMPLEX_H_
#define _BBM_DRJIT_COMPLEX_H_

#include <ostream>
#include "drjit/complex.h"
#include "util/toString.h"

/************************************************************************/
/*! \file complex.h

  \brief Define complex as an alias to drjit::Complex. Also define a custom
  toString method to mimic the native version's complex output.

*************************************************************************/

namespace backbone {

  /**********************************************************************/
  /*! \brief Alias drjit::Complex
   **********************************************************************/
  template<typename T>
    using complex = drjit::Complex<T>;

  /**********************************************************************/
  /*! @{ alias external methods
   **********************************************************************/
  using drjit::real;
  using drjit::imag;
  using drjit::conj;
  //! @}

  //! \brief delete atan2 to avoid static_error
  template<typename T, typename U> auto atan2(drjit::Complex<T> a, drjit::Complex<U> b) = delete;
  
  /**********************************************************************/
  /*! \brief Custom toString
   **********************************************************************/
  template<typename T>
    inline std::string toString(const complex<T>& c)
  {
    return std::string("[") + bbm::toString(backbone::real(c)) + ", " + bbm::toString(backbone::imag(c)) + "]";
  }
  
} // end backbone namespace

#endif /* _BBM_DRJIT_COMPLEX_H_ */
