#ifndef _BBM_ENOKI_COMPLEX_H_
#define _BBM_ENOKI_COMPLEX_H_

#include "enoki/complex.h"

/************************************************************************/
/*! \file complex.h

  \brief Define complex as an alias to enoki::Complex.  Also define a
  custom toString method to mimic the native version's complex output.

************************************************************************/

namespace backbone {

  /**********************************************************************/
  /*! \brief Alias enoki::Complex
   **********************************************************************/
  template<typename T>
    using complex = enoki::Complex<T>;
  
  /*********************************************************************/
  /*! @{ alias external methods
   *********************************************************************/
  using enoki::real;
  using enoki::imag;
  using enoki::conj;
  //! @}

} // end backbone namespace  

#endif /* _BBM_ENOKI_COMPLEX_H_ */
