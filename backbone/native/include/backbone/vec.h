#ifndef _BBM_NATIVE_VEC_H_
#define _BBM_NATIVE_VEC_H_

#include "backbone/array.h"

/************************************************************************/
/*! \file vec.h

  \brief Defines vec2d and vec3d as specializations of an array

************************************************************************/

namespace backbone {

  template<typename T>
    using vec2d = array<T, 2>;

  template<typename T>
    using vec3d = array<T, 3>;

} // end backbone namespace

#endif /* _BBM_NATIVE_VEC_H_ */
