#ifndef _BBM_SHADINGFRAME_H_
#define _BBM_SHADINGFRAME_H_

#include "core/mat.h"
#include "core/vec.h"

/************************************************************************/
/*! \file shading_frame.h
    \brief Shading frame transformations
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Construct a local shading frame to global frame transformation
      given a normal direction

    \param normal = local shading frame normal direction
    \returns shading frame transformation matrix from local to global frame.

    Create a shading frame transformation from the local to the global shading
    frame determined by the surface normal. The tangent vector is randomly
    selected.
  ***********************************************************************/
  template<typename T>
    mat3d<std::decay_t<T>> toGlobalShadingFrame(const vec3d<T>& normal)
  {
    using value_t = std::decay_t<T>;
      
    // normal == Z axis
    vec3d<value_t> X, Y, Z = bbm::normalize(normal);

    // Implements: Duff et al. "Building an Orthonormal Basis, Revisited", Journal of Computer Graphics, Vol. 6, No. 1, 2017
    value_t sign = bbm::sign(vec::z(Z));
    const value_t a = -1.0 / (sign + vec::z(Z));
    const value_t b = vec::x(Z) * vec::y(Z) * a;
    
    X = vec3d<value_t>(1.0 + sign * vec::x(Z) * vec::x(Z) * a,
                       sign * b,
                       -sign * vec::x(Z));

    Y = vec3d<value_t>(b,
                       sign + vec::y(Z) * vec::y(Z) * a,
                       -vec::y(Z));

    // done.
    return mat3d<value_t>(X, Y, Z);
  }

  
  /**********************************************************************/
  /*! \brief Construct a global to local shading frame transformation
   **********************************************************************/
  template<typename T>
  inline auto toLocalShadingFrame(const vec3d<T>& normal)
  {
    return transpose(toGlobalShadingFrame(normal));
  }
  
} // end bbm namespace

#endif /* _BBM_SHADINGFRAME_H_ */
