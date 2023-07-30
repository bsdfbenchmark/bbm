#ifndef _BBM_HORIZONTAL_BACKBONE_CONCEPT_H_
#define _BBM_HORIZONTAL_BACKBONE_CONCEPT_H_

#include <concepts>

/************************************************************************/
/*! \file horizontal.h

  \brief Horizontal operators (aggregation over all elements in the type)

  Assumes: backbone type traits are already defined!
  
*************************************************************************/

namespace bbm {
  namespace concepts {
    namespace backbone {

      /******************************************************************/
      /*! \brief A type T has horizontal functions if:

        Regular horizontal operations:
        + hsum(T a): returns value_t(a[0] + a[1] + ...)
        + hprod(T a): returns value_t(a[0] * a[1] * ...)
        + hmax(T a): returns the maximum element value_t
        + hmin(T a): returns the minimum element value_t
        + dot(T a, T b): returns value_t( a[0]*b[0], a[1]*b[1], ....)
        + norm(T a): return sqrt(dot(a,a))
        + squared_norm(T a): returns dot(a,a)
        + normalize(T a): returns (a / norm(a))
        
        Mask horizontal operations:
        + all(M a): (a[0] && a[1] && ...)
        + any(M a): (a[0] || a[1] || ...)
        + none(M a): !all(a)
        + count(M a): (a[0] + a[1] + ....)
      *******************************************************************/
      template<typename T>
      concept horizontal = requires(T a)
      {
        { bbm::hsum(a) } -> std::convertible_to<value_t<T>>;
        { bbm::hprod(a) } -> std::convertible_to<value_t<T>>;
        { bbm::hmax(a) } -> std::convertible_to<value_t<T>>;
        { bbm::hmin(a) } -> std::convertible_to<value_t<T>>;
        { bbm::dot(a,a) } -> std::convertible_to<value_t<T>>;
        { bbm::norm(a) } -> std::convertible_to<value_t<T>>;
        { bbm::squared_norm(a) } -> std::convertible_to<value_t<T>>;
        { bbm::normalize(a) } -> std::convertible_to<T>;
      };


      /******************************************************************/
      /*! \brief A type M has horizontal masking functions if:
        + M is a recognized mask type
        + all(M) returns a bool(a[0] && a[1] && ...)
        + any(M) returns a bool(a[0] || a[1] || ...)
        + none(M) returns a bool(!any(M))
        + count(M) returns a size_t(hsum(M))
      *******************************************************************/
      template<typename M>
        concept horizontal_mask = requires(M a)
      {
        { bbm::is_mask_v<M> } -> std::convertible_to<bool>;
        { bbm::all(a) } -> std::convertible_to<bool>;
        { bbm::any(a) } -> std::convertible_to<bool>;
        { bbm::none(a) } -> std::convertible_to<bool>;
        { bbm::count(a) } -> std::convertible_to<size_t>;

        requires is_mask_v<M>;
      };
      
    } // end backbone namespace
  } // end concepts namespace
} // end bbm namespace


#endif /* _BBM_HORIZONTAL_BACKBONE_CONCEPT_H_ */
