#ifndef _BBM_ENOKI_VEC_H_
#define _BBM_ENOKI_VEC_H_

#include "backbone/array.h"

/***********************************************************************/
/*! \file vec.h

  \brief Defines 2D, and 3D vectors as aliases to enoki arrays.
************************************************************************/
  
namespace backbone {
  
    template<typename T>
      using vec2d = enoki::Array<T, 2>;

    template<typename T>
      using vec3d = enoki::Array<T, 3>;

} // end backbone namespace

#endif /* _BBM_ENOKI_VEC_H_ */
