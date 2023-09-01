#ifndef _BBM_VEC_H_
#define _BBM_VEC_H_

#include "util/reference.h"

/************************************************************************/
/*! \file vec.h

  \brief Defines additional helper methods for vectors.
  
*************************************************************************/
  

namespace bbm {
  namespace vec {

    /********************************************************************/
    /*! @{ \name Shortcuts to vector coordinates
     ********************************************************************/
    template<typename T> inline constexpr decltype(auto) x(bbm::vec3d<T>& v) { return v[0]; }
    template<typename T> inline constexpr decltype(auto) x(const bbm::vec3d<T>& v) { return v[0]; }

    template<typename T> inline constexpr decltype(auto) y(bbm::vec3d<T>& v) { return v[1]; }
    template<typename T> inline constexpr decltype(auto) y(const bbm::vec3d<T>& v) { return v[1]; }

    template<typename T> inline constexpr decltype(auto) z(bbm::vec3d<T>& v) { return v[2]; }
    template<typename T> inline constexpr decltype(auto) z(const bbm::vec3d<T>& v) { return v[2]; }

    /***/
    
    template<typename T> inline constexpr decltype(auto) x(bbm::vec2d<T>& v) { return v[0]; }
    template<typename T> inline constexpr decltype(auto) x(const bbm::vec2d<T>& v) { return v[0]; }

    template<typename T> inline constexpr decltype(auto) y(bbm::vec2d<T>& v) { return v[1]; }
    template<typename T> inline constexpr decltype(auto) y(const bbm::vec2d<T>& v) { return v[1]; }

    template<typename T> inline constexpr decltype(auto) u(bbm::vec2d<T>& v) { return v[0]; }
    template<typename T> inline constexpr decltype(auto) u(const bbm::vec2d<T>& v) { return v[0]; }

    template<typename T> inline constexpr decltype(auto) v(bbm::vec2d<T>& v) { return v[1]; }
    template<typename T> inline constexpr decltype(auto) v(const bbm::vec2d<T>& v) { return v[1]; }
    //! @}

    /********************************************************************/
    /*! @{ \name Reduce vec3d to vec2d
     ********************************************************************/
    template<typename T> inline constexpr const vec2d<T> xy(const vec3d<T>& v) { return {v[0], v[1]}; }
    template<typename T> inline constexpr const vec2d<T> xz(const vec3d<T>& v) { return {v[0], v[2]}; }
    template<typename T> inline constexpr const vec2d<T> yz(const vec3d<T>& v) { return {v[1], v[2]}; }
    //! @}
    
    /********************************************************************/
    /*! @{ \name Increase the dimension of a vec
     ********************************************************************/
    template<typename T, typename V> inline constexpr const vec3d<T> expand(const vec2d<T>& v, V&& a) { return vec3d<T>(v[0], v[1], a); }
    template<typename T> inline constexpr const vec3d<T> expand(const vec2d<T>& v) { return vec3d<T>(v[0], v[1], T(bbm::safe_sqrt(1 - bbm::squared_norm(v)))); }
    template<typename T> inline constexpr const vec2d<T> expand(T&& c) { return vec2d<T>(c, bbm::safe_sqrt(1 - c*c)); }
    //! @}

  } // end vec namespace

} // end bbm namespace

#endif /* _BBM_VEC_H_ */
