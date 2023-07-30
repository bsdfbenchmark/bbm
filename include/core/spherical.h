#ifndef _BBM_SPHERICAL_H_
#define _BBM_SPHERICAL_H_

#include "core/vec.h"
#include "core/constants.h"
#include "core/transform.h"

/***********************************************************************/
/*! \file spherical.h
  \brief Methods for handling spherical coordinates

  Methods for working with spherical coordinates stored in a Vec2d,
  including conversion methods to Vec3d and common trig operations.
************************************************************************/

namespace bbm {

  namespace spherical {
    
    ////////////////////////////
    //! @{ \name Theta access
    ////////////////////////////
    template<typename T> T& theta(vec2d<T>& v)             { return v[1]; }
    template<typename T> const T& theta(const vec2d<T>& v) { return v[1]; }

    template<typename T> T theta(const vec3d<T>& v)
    {
      auto diff(v);
      vec::z(diff) -= bbm::sign(vec::z(v));
      auto temp = 2.0 * bbm::asin(0.5 * bbm::norm(diff));
      return bbm::select( vec::z(v) >= 0, temp, constants<T>::Pi() - temp );
    }
    //! @}
    

    //////////////////////////
    //! @{ \name Phi access
    //////////////////////////
    template<typename T> T& phi(vec2d<T>& v)             { return v[0]; }
    template<typename T> const T& phi(const vec2d<T>& v) { return v[0]; }

    template<typename T> T phi(const vec3d<T>& v)
    {
      T result = atan2( vec::y(v), vec::x(v) );
      return bbm::select(result < 0, result + constants<T>::Pi(2), result);
    }
    //! @}

    ///////////////////////////
    //! @{ \name Conversion 
    ///////////////////////////
    template<typename T>
      vec2d<T> convert(const vec3d<T>& v)
      {
        return vec2d<T>(phi(v), theta(v));
      }

    template<typename T>
      vec3d<T> convert(const vec2d<T>& v)
      {
        vec2d<T> cst = bbm::cossin(theta(v));
        vec2d<T> csp = bbm::cossin(phi(v));

        return vec::expand(csp*cst[1], cst[0]);
      }
    //! @}
    
    
    ////////////////////////////////////////
    // Shorthand Trigionometric Functions //
    ////////////////////////////////////////
    
    //! @{ \name Sine variants
    template<typename T>
      T sinTheta(const vec2d<T>& v) { return bbm::sin(theta(v)); }
    template<typename T>
      T sinTheta2(const vec2d<T>& v) { auto t = sinTheta(v); return t*t; }

    template<typename T>
    T sinTheta2(const vec3d<T>& v) { return bbm::max(1 - vec::z(v)*vec::z(v), 0); }

    template<typename T>
      T sinTheta(const vec3d<T>& v) { return bbm::sqrt(sinTheta2(v)); }

    template<typename T>
      T sinPhi(const vec2d<T>& v) { return bbm::sin(phi(v)); }

    template<typename T>
      T sinPhi(const vec3d<T>& v)
      {
        auto sT = sinTheta(v);
        return bbm::select(bbm::abs(sT) < constants<T>::Epsilon(), 0, bbm::clamp(vec::y(v) / sT, -1, +1));
      }

    template<typename T>
      T sinPhi2(const vec2d<T>& v) { auto t = sinPhi(v); return t*t; }

    template<typename T>
      T sinPhi2(const vec3d<T>& v)
      {
        auto sT2 = sinTheta2(v);
        return bbm::select(bbm::abs(sT2) < constants<T>::Epsilon, 0, bbm::clamp(vec::y(v)*vec::y(v) / sT2, 0, +1));
      }
    //! @}

    
    //! @{ \name Cosine variants
    template<typename T>
      T cosTheta(const vec2d<T>& v) { return bbm::cos(theta(v)); }

    template<typename T>
      T cosTheta(const vec3d<T>& v) { return vec::z(v); }

    template<typename V>
      bbm::value_t<V> cosTheta2(const V& v) { auto t = cosTheta(v); return t*t; }

    template<typename T>
      T cosPhi(const vec2d<T>& v) { return bbm::cos(phi(v)); }

    template<typename T>
      T cosPhi(const vec3d<T>& v)
      {
        auto sT = sinTheta(v);
        return bbm::select(bbm::abs(sT) < constants<T>::Epsilon(), +1, bbm::clamp(vec::x(v) / sT, -1, +1));
      }

    template<typename T>
      T cosPhi2(const vec2d<T>& v) { auto t = cosPhi(v); return t*t; }

    template<typename T>
      T cosPhi2(const vec3d<T>& v)
      {
        auto sT2 = sinTheta(v);
        return bbm::select(bbm::abs(sT2) < constants<T>::Epsilon(), +1, bbm::clamp(vec::x(v)*vec::x(v) / sT2, 0, +1));
      }
    //! @}


    //! @{ \name Joint Cos/Sin variants
    template<typename T>
      vec2d<T> cossinTheta(const vec2d<T>& v) { return bbm::cossin(theta(v)); }

    template<typename T>
      vec2d<T> cossinTheta(const vec3d<T>& v) { return vec2d<T>(cosTheta(v), sinTheta(v)); }

    template<typename T>
      vec2d<T> cossinTheta2(const vec2d<T>& v) { auto t = cossinTheta(v); return t*t; }

    template<typename T>
      vec2d<T> cossinTheta2(const vec3d<T>& v) { return vec2d<T>(sinTheta2(v), cosTheta2(v)); }

    template<typename T>
      vec2d<T> cossinPhi(const vec2d<T>& v) { return bbm::cossin(phi(v)); }

    template<typename T>
      vec2d<T> cossinPhi(const vec3d<T>& v)
    {
      auto sT = sinTheta(v);
      auto rsT = bbm::rcp(sT);
      return bbm::select(bbm::abs(sT) < constants<T>::Epsilon(), vec2d<T>(+1,0), bbm::clamp(vec::xy(v) * rsT, -1, +1));
    }

    template<typename T>
      vec2d<T> cossinPhi2(const vec2d<T>& v) { auto t = cossinPhi(v); return t*t; }

    template<typename T>
      vec2d<T> cossinPhi2(const vec3d<T>& v)
    {
      auto sT2 = sinTheta2(v);
      auto rsT2 = bbm::rcp(sT2);
      return bbm::select(bbm::abs(sT2) < constants<T>::Epsilon(), vec2d<T>(+1,0), bbm::clamp(vec::xy(v)*vec::xy(v) * rsT2, -1, +1));
    }
    //! @}
    
    //! @{ \name Tangent variants
    template<typename T>
      T tanTheta(const vec2d<T>& v) { return bbm::tan(theta(v)); }
    
    template<typename T>
    T tanTheta(const vec3d<T>& v) { return sinTheta(v) / cosTheta(v); }
    
    template<typename T>
      T tanTheta2(const vec2d<T>& v) { auto t = tanTheta(v); return t*t; }
    
    template<typename T>
      T tanTheta2(const vec3d<T>& v) { return sinTheta2(v) / cosTheta2(v); }
    
    template<typename T>
      T tanPhi(const vec2d<T>& v) { return bbm::tan(phi(v)); }
    
    template<typename T>
      T tanPhi(const vec3d<T>& v) { return vec::y(v) / vec::x(v); }
    
    template<typename T>
      T tanPhi2(const T& v) { auto t = tanPhi(v); return t*t; }
    //! @}
        
  } // end namespace spherical
}   // end namespace bbm

#endif /* _BBM_SPHERICAL_H_ */
