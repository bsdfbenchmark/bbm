#ifndef _BBM_ENOKI_ARRAY_H_
#define _BBM_ENOKI_ARRAY_H_

#include "enoki/array.h"

/************************************************************************/
/*! \file array.h

  \brief Additions to enoki::Array

*************************************************************************/

template<typename T>  requires enoki::is_array_v<T>
  inline auto& operator++(T& a) { a += 1; return a; }

template<typename T> requires enoki::is_array_v<T>
  inline auto operator++(const T& a, int) { auto ret = a + 1; return ret; }

template<typename T> requires enoki::is_array_v<T>
  inline auto& operator--(T& a) { a -= 1; return a; }

template<typename T> requires enoki::is_array_v<T>
  inline auto operator--(const T& a, int) { auto ret = a - 1; return ret; }
  
#endif /* _BBM_ENOKI_ARRAY_H_ */
