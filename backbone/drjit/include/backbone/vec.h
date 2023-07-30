#ifndef _BBM_DRJIT_VEC_H_
#define _BBM_DRJIT_VEC_H_

/************************************************************************/
/*! \file vec.h

  \brief Defines 2D and 3D vectors as aliases to drjit arrays.
*************************************************************************/

namespace backbone {

  template<typename T>
    using vec2d = drjit::Array<T,2>;

  template<typename T>
    using vec3d = drjit::Array<T,3>;

} // end backbone namespace

#endif /* _BBM_DRJIT_VEC_H_ */
