#ifndef _BBM_VEC_TRANSFORM_H_
#define _BBM_VEC_TRANSFORM_H_

#include "core/spherical.h"

/************************************************************************/
/*! \file vec_transform.h

  \brief Vector transformation operations

*************************************************************************/

namespace bbm {

    /********************************************************************/
    /*! \brief Returns the lefthand (clockwise) perpendicular vector of a 2D
        vector.

      \param v       Input vector
      \returns       (y, -x)
    *********************************************************************/
    template<typename T>
      inline vec2d<T> perp(const vec2d<T>& v) { return vec2d<T>(vec::y(v), -vec::x(v)); }

  
    /********************************************************************/
    /*! \brief Returns the righthand (counterclockwise) perpendicular vector of a 2D
        vector.

      \param v       Input vector
      \returns       (-y, x)
    *********************************************************************/
    template<typename T>
      inline vec2d<T> cperp(const vec2d<T>& v) { return vec2d<T>(-vec::y(v), vec::x(v)); }

  
    /********************************************************************/
    /*! \brief Reflects a 3D vector.
  
      \param v       Input vector to reflect
      \param normal  Normal to reflect around
    *********************************************************************/
    template<typename T>
      inline vec3d<T> reflect(const vec3d<T>& v, const vec3d<T>& normal) { return normal * dot(normal, v) * 2.0 - v; }

    /********************************************************************/
    /*! \brief Reflects a 3D vector around Z=1.
  
      \param v       Input vector to reflect
    *********************************************************************/
    template<typename T>
      inline vec3d<T> reflect(const vec3d<T>& v) { return vec3d<T>(-vec::x(v), -vec::y(v), vec::z(v)); }


    /********************************************************************/
    /*! \brief Cross product of two 3D vectors.

      \param a = first vector
      \param b = second vector
     ********************************************************************/
    template<typename T>
      inline constexpr vec3d<T> cross(const vec3d<T>& a, const vec3d<T>& b)
    {
      return vec3d<T>( a[1]*b[2] - a[2]*b[1],
                       a[2]*b[0] - a[0]*b[2],
                       a[0]*b[1] - a[1]*b[0] );
    }

  
    /********************************************************************/
    /*! \brief Halfway vector (3D)

      \param a = first vector
      \param b = second vector
    ********************************************************************/
    template<typename T>
      vec3d<T> halfway(const vec3d<T>& a, const vec3d<T>& b)
    {
      return bbm::normalize(a + b);
    }

    
    /********************************************************************/
    /*! \brief Convert fro Canonical to the Halfway-Difference
        parameterization

      \param a = first vector
      \param b = second vector
    *********************************************************************/
    template<typename T>
      std::pair<vec3d<T>, vec3d<T>> convertToHalfwayDifference(const vec3d<T>& a, const vec3d<T>& b)
    {
      vec3d<T> half = halfway(a,b);
      vec2d<T> half_sph = spherical::convert(half);
      vec3d<T> diff = rotationY(-spherical::theta(half_sph)) * (rotationZ(-spherical::phi(half_sph)) * a);
      return {half, diff};
    }

    /********************************************************************/
    /*! \brief Difference vector

      \param a = first vector
      \param b = second vector
    *********************************************************************/
    template<typename T>
      vec3d<T> difference(const vec3d<T>& a, const vec3d<T>& b)
    {
      return convertToHalfwayDifference(a,b).second;
    }

    /********************************************************************/
    /*! \brief Convert from Halfway-Difference to Canonical parameterization

      \param a = first vector
      \param b = second vector
    *********************************************************************/
    template<typename T>
      std::pair<vec3d<T>, vec3d<T>> convertFromHalfwayDifference(const vec3d<T>& half, const vec3d<T>& diff)
    {
      auto in = rotationZ(phi(half)) * (rotationY(theta(half)) * diff);
      auto out = rotationZ(phi(half)) * (rotationY(theta(half)) * vec3d<T>(-vec::x(diff), -vec::y(diff), vec::z(diff)));
      return {in, out}; 
    }


} // bbm namespace

#endif /* _BBM_VEC_TRANSFORM_H_ */
