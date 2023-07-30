#ifndef _BBM_TRANSFORM_H_
#define _BBM_TRANSFORM_H_

#include "core/vec.h"
#include "core/mat.h"

/***********************************************************************/
/*! \file transform.h
  \brief Basic matrix transformations
************************************************************************/

namespace bbm {

    /*******************************************************************/
    /*! \brief Rotation around the X-axis

      \param angle = rotation angle
    ********************************************************************/
    template<typename T>
      mat3d<std::decay_t<T>> rotationX(const vec2d<T>& cossin)
    {
      using matrix_t = mat3d<std::decay_t<T>>;
      matrix_t result( vec3d<T>(1,0,0),
                       vec3d<T>(0, cossin[0], cossin[1]),
                       vec3d<T>(0, -cossin[1], cossin[0]) );
      return result;
    }

    /*******************************************************************/
    /*! \brief Rotation around the X-axis

      \param angle = rotation angle
    ********************************************************************/
    template<typename T>
      mat3d<std::decay_t<T>> rotationX(T angle)
    {
      auto cossin = bbm::cossin(angle);
      return rotationX(cossin);
    }
  
    /*******************************************************************/
    /*! \brief Rotation around the Y-axis

      \param angle = rotation angle
    ********************************************************************/
    template<typename T>
      mat3d<std::decay_t<T>> rotationY(const vec2d<T>& cossin)
    {
      using matrix_t = mat3d<std::decay_t<T>>;
      matrix_t result( vec3d<T>(cossin[0], 0, -cossin[1]),
                       vec3d<T>(0,1,0),
                       vec3d<T>(cossin[1], 0, cossin[0]) );
      return result;
    }

    /*******************************************************************/
    /*! \brief Rotation around the Y-axis

      \param angle = rotation angle
    ********************************************************************/
    template<typename T>
      mat3d<std::decay_t<T>> rotationY(T angle)
    {
      auto cossin = bbm::cossin(angle); 
      return rotationY(cossin);
    }
      
    /*******************************************************************/
    /*! \brief Rotation around the Z-axis

      \param angle = rotation angle
    ********************************************************************/
    template<typename T>
      mat3d<std::decay_t<T>> rotationZ(const vec2d<T>& cossin)
    {
      using matrix_t = mat3d<std::decay_t<T>>;
      matrix_t result( vec3d<T>(cossin[0], cossin[1], 0),
                       vec3d<T>(-cossin[1], cossin[0], 0),
                       vec3d<T>(0,0,1) );
      return result;
    }

    /*******************************************************************/
    /*! \brief Rotation around the Z-axis

      \param angle = rotation angle
    ********************************************************************/
    template<typename T>
      mat3d<std::decay_t<T>> rotationZ(T angle)
    {
        auto cossin = bbm::cossin(angle); 
        return rotationZ(cossin);
    }
        
    /********************************************************************/
    /*! \brief 2D rotation matrix

      \param cossin = (cos, sin)
    ********************************************************************/
    template<typename T>
      inline mat2d<std::decay_t<T>> rotation2d(const vec2d<T>& cossin)
    {
      using matrix_t = mat2d<std::decay_t<T>>;
      matrix_t result( cossin, vec2d<T>(-cossin[1], cossin[0]) );
      return result;
    }
  
    /********************************************************************/
    /*! \brief 2D rotation matrix

      \param angle = rotation angle
    ********************************************************************/
    template<typename T>
      mat2d<std::decay_t<T>> rotation2d(T angle)
    {
      auto cossin = bbm::cossin(angle);
      return rotation2d(cossin);
    }
  
} // end bbm namespace

#endif /* _BBM_TRANSFORM_H_ */
