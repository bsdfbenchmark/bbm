#ifndef _BBM_PRECOMPUTE_H_
#define _BBM_PRECOMPUTE_H_

#include <array>
#include <numeric>
#include "concepts/util.h"
#include "util/named.h"
#include "util/constfor.h"
#include "core/error.h"

/************************************************************************/
/*! \file precompute.h

  \brief Helper structures and methods for storing precomputed compiled data 

*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief std::array wrapper for precomputed data with optional remapping of the indices.

    \tparam T = underlying data type
    \tparam DIM = array of size of each dimension. E.g., {2,3} yields a 2 x 3
                  data structure, where '3' is the inner dimension.

    \tparam MAP = either left empty, or a functor that maps the coordinates to
       the indices. The number of functors needs to be either '0' (all are
       identity mappings) or DIM.size().  The functors, if provided, can take
       one coordinate at the time and map it to an index (one index per MAP),
       or it can take all coordinates at once, and map it to an index (one
       index per MAP).  In both cases, the number of MAPs needs to equal
       to DIM.size().
  ***********************************************************************/
    
  template<typename T, std::array DIM, typename... MAP> requires (sizeof...(MAP) == DIM.size() || sizeof...(MAP) == 0) 
    struct tab : public std::array<T, std::accumulate(std::begin(DIM), std::end(DIM), 1, std::multiplies<>())>
  {
    using base_type = std::array<T, std::accumulate(std::begin(DIM), std::end(DIM), 1, std::multiplies<>())>;
  public:
    //! \brief value type
    using value = T;
    
    //! \brief sizes of each dimension.
    static constexpr auto sizes(void) { return DIM; }

    //! \brief number of dimensions.
    static constexpr auto dim(void) { return DIM.size(); }

  private:
    /********************************************************************/
    /*! \brief PRIVATE method to map a coordinate to an array of indices.
      *******************************************************************/
    template<typename... Cs> requires (sizeof...(Cs) == dim())
      static constexpr auto map(Cs&&... coord)
    {
      // Case 1: No MAP has been given; assume coord == index
      if constexpr (sizeof...(MAP) == 0)
      {
        using type = decltype((coord + ...));
        return std::array{ bbm::cast<type>(coord)...};
      }
      else
      {
        // Case 2: MAP uses all coord... at once to produce an index
        if constexpr ((concepts::lambda<MAP, Cs...> && ...))
        {
          using type = decltype((MAP()(coord...) + ...));
          return std::array{ bbm::cast<type>(MAP()(coord...))... };
        }

        // Case 3: MAP only uses a single coord at the time to produce an index
        else if constexpr((concepts::lambda<MAP, Cs> && ...))
        {
          using type = decltype((MAP()(coord) + ...));
          return std::array{ bbm::cast<type>(MAP()(coord))... };
        }

        // Otherwise invalid signature
        else static_assert(dependent_false_v<MAP...>, BBM_INVALID_LAMBDA);
      }
    }

    /********************************************************************/
    /*! \brief PRIVATE: linearizes an array of indices (per dimension) to a
        single index in the underlying std::array. Indices outside the range
        are clamped to 0 or DIM-1.
     ********************************************************************/
    template<typename I>
    static constexpr auto index(const std::array<I,dim()>& mapped)
    {
      index_t<I> idx = 0;
      CONSTFOR(i, dim(),
      {
        idx = bbm::cast<index_t<I>>(bbm::clamp(mapped[i],0,DIM[i]-1)) + idx*DIM[i];
      });
      return idx;
    }

    /********************************************************************/
    /*! \brief PRIVATE: returns false if any index lies outside the
        corresponding dimension's size.
     ********************************************************************/
    template<typename I>
      static constexpr auto valid(const std::array<I,dim()>& mapped)
    {
      index_mask_t<I> mask = true;
      CONSTFOR(i, dim(), { mask &= bbm::cast<index_mask_t<I>>((mapped[i] < DIM[i]) && (mapped[i] >= 0)); });
      return mask;
    }

    /********************************************************************/
    /*! \brief PRIVATE: given an array of indices and a mask, return the
        corresponding stored value and update the mask if the requested value
        lies inside the underlying std::array (true) or outside (false). When
        'false' is return the lookup value will be clamped to lie inside the
        data-range.
     ********************************************************************/
    template<typename RET, typename I, typename Mask> requires std::convertible_to<index_mask_t<I>, Mask>
      inline constexpr auto _lookup(const std::array<I,dim()>& mapped, Mask& mask) const
    {
      mask &= valid(mapped);
      auto idx = index(mapped);
      return bbm::lookup<RET>(*this, idx);  // all are made valid
    }

    /********************************************************************/
    /*! \brief PRIVATE: Similar to _lookup, except that the each value is the
        interpolation result of the floor and ceil of each index. (e.g.,
        linear interpolation in 1D, bilinear in 2D, etc...)
     ********************************************************************/
    template<typename RET, size_t IDX, typename A, typename Mask>
      inline constexpr RET _interpolate(const A& mapped, Mask& mask) const
    {
      auto a_idx(mapped); a_idx[IDX] = bbm::floor(mapped[IDX]);
      auto b_idx(mapped); b_idx[IDX] = bbm::ceil(mapped[IDX]);      
      auto weight = mapped[IDX] - bbm::floor(mapped[IDX]);

      if constexpr (IDX == DIM.size()-1) return bbm::lerp( _lookup<RET>(a_idx, mask), _lookup<RET>(b_idx, mask), weight );
      else return bbm::lerp( _interpolate<RET,IDX+1>(a_idx, mask), _interpolate<RET,IDX+1>(b_idx, mask), weight );
    }
      
  public:
    /********************************************************************/
    /*! @{ \name Direct Access by indices (per dimension); throws exception if out of range!
      *******************************************************************/
    template<typename... Is> requires (sizeof...(Is) == dim())
      inline T& operator()(Is&&... indices)  { return base_type::operator[]( index(std::array<size_t, dim()>{size_t(indices)...})); }
    
    template<typename... Is> requires (sizeof...(Is) == dim())
      inline const T& operator()(Is&&... indices)  const { return base_type::operator[](index(std::array<size_t, dim()>{size_t(indices)...})); }
    //! @}
    
    /********************************************************************/
    /*! \brief Lookup the value in the data structure given a set of
        coordinates for each dimension. Uses nearest neighbor lookup if the
        coordinates do not exactly correspond to an integer position in the
        data structure.

        \tparam RET = return type (must be specified)
        \param coords = a coordinate per dimension
        \returns a named tuple, with the resulting lookup "value" and a "valid"
                 indicating whether the value is valid.

        Note: the coordinate is first converted to indices with the MAP functors.
    *********************************************************************/
    template<typename RET, typename... Cs> requires (sizeof...(Cs) == dim())
      inline constexpr auto lookup(Cs&&... coords) const
    {
      mask_t<RET> mask = true;
      RET value = _lookup<RET>( map(std::forward<Cs>(coords)...), mask );
      return bbm::make_named<"value", "valid">(value, mask);
    }

    //! \brief Lookup with the native type
    template<typename... Cs> requires (sizeof...(Cs) == dim())
      inline constexpr auto lookup(Cs&&... coords) const { return lookup<T>(std::forward<Cs>(coords)...); }
    
    /********************************************************************/
    /*! \brief Lookup the value in the data structure given a set of
      coordinates.  In contrast to 'lookup' this method will linearly
      interpolate the values of the corresponding neighoring integer indices.
      In 1D this is just a linear interpolation, in 2D this corresponds to a
      bilinear interpolation, etc...  Thus DIM^2 lookup are interpolated.
      
      \tparam RET = return type (must be specified)
      \param coords = a coordinate per dimension
      \returns a named tuple, with the resulting interpolated lookup "value"
                 and a "valid" indicating whether the value is valid.

      Note: the coordinate is first converted to indices with the MAP functors.
    *********************************************************************/
    template<typename RET, typename... Cs> requires (sizeof...(Cs) == dim())
      inline constexpr auto interpolate(Cs&&... coords) const
    {
      mask_t<RET> mask = true;
      RET value = _interpolate<RET,0>( map(std::forward<Cs>(coords)...), mask );
      return bbm::make_named<"value", "valid">(value, mask);
    }

    //! \brief interpolate with the native type
    template<typename... Cs> requires (sizeof...(Cs) == dim())
      inline constexpr auto interpolate(Cs&&... coords) const { return interpolate<T>(std::forward<Cs>(coords)...); }
  };

} // end bbm namespace

#endif /* _BBM_PRECOMPUTE_H_ */
