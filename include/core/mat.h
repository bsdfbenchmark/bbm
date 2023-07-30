#ifndef _BBM_MAT_H_
#define _BBM_MAT_H_

#include <ostream>

#include "core/vec.h"
#include "util/multirange_for.h"

/************************************************************************/
/*! \file matrix.h

  \brief Defines mat2d and mat3d on top of vec2d and vec3d

************************************************************************/

namespace bbm {

  /*** Implementation details for matrices ***/
  namespace detail {

    template<typename Vec, size_t N>
      struct mat
    {
      using value = value_t<Vec>;
      
      //! \brief Default constructor
      inline mat(void) : _col() {};

      //! \brief Set the diagonal to val
      inline mat(value&& val)
      {
        for(size_t i=0; i != N; ++i)
        {
          _col[i] = 0;
          _col[i][i] = val;
        }
      }

      //! brief Set each column
      template<typename... V> requires (sizeof...(V) == N)
        inline mat(V&&... col) : _col({(std::forward<V>(col))...}) {};

      //! \brief Copy constructor
      inline mat(const mat& src) : _col(src._col) {}

      //! @{ \name Lookup operator
      decltype(auto) operator()(size_t row, size_t col) const { return _col[col][row]; }
      decltype(auto) operator()(size_t row, size_t col) { return _col[col][row]; }

      decltype(auto) col(size_t col) const { return _col[col]; }
      Vec row(size_t row) const
      {
        Vec result;
        for(size_t i=0; i != N; ++i)
          result[i] = _col[i][row];
        return result;
      }
      //! @}

      //! @{ Matrix Methods
      friend mat transpose(const mat& m)
      {
        mat result;
        for(size_t row=0; row != N; ++row)
          for(size_t col=0; col != N; ++col)
            result(col,row) = m(row,col);
        return result;
      }
      //! @}

      //! \brief Print to ostream
      friend std::ostream& operator<<(std::ostream& s, const mat& m)
      {
        s << "[";
        for(size_t i=0; i != N; ++i)
        {
          if(i != 0) s << ", ";
          s << m.row(i);
        }
        s << "]";
        return s;
      }
      
      //! @{ \name Math Operators
      inline mat operator-(void) const { return apply_op(std::negate<>()); }
      
      inline mat operator+(const mat& m) const { return apply_op(std::plus<>(), m._col); }
      inline mat operator-(const mat& m) const { return apply_op(std::minus<>(), m._col); }

      inline mat& operator+=(const mat& m) { return apply_op_inplace( [](auto& r, auto& m) { r += m; }, m); }
      inline mat& operator-=(const mat& m) { return apply_op_inplace( [](auto& r, auto& m) { r -= m; }, m); }
      
      inline mat operator+(const value& v) const { return apply_op(std::plus<>(), v); }
      inline mat operator-(const value& v) const { return apply_op(std::minus<>(), v); }
      inline mat operator*(const value& v) const { return apply_op(std::multiplies<>(), v); }
      inline mat operator/(const value& v) const { return apply_op(std::divides<>(), v); }

      inline mat& operator+=(value&& v) { return apply_op_inplace([](auto& r, auto& v) { r+= v; }, v); }
      inline mat& operator-=(value&& v) { return apply_op_inplace([](auto& r, auto& v) { r-= v; }, v); }
      inline mat& operator*=(value&& v) { return apply_op_inplace([](auto& r, auto& v) { r*= v; }, v); }
      inline mat& operator/=(value&& v) { return apply_op_inplace([](auto& r, auto& v) { r/= v; }, v); }
      
      friend inline constexpr mat operator+(const value& v, const mat& m) { return (m+v); }
      friend inline constexpr mat operator-(const value& v, const mat& m) { return (-m + v); }
      friend inline constexpr mat operator*(const value& v, const mat& m) { return (m*v); }

      inline Vec operator*(const Vec& v) const
      {
        Vec result;
        for(size_t r=0; r != N; ++r)
        {
          auto row = this->row(r);
          result[r] = dot(row, v);
        }
        return result;
      }

      inline mat operator*(const mat& m) const
      {
        mat result;
        for(size_t r=0; r != N; ++r)
        {
          auto row = this->row(r);
          for(size_t c=0; c != N; ++c)
            result(r, c) = dot(row, m.col(c));
        }
        return result;
      }

      inline mat& operator*=(const mat& m)
      {
        *this = *this * m;
        return *this;
      }
      //! @}
      
    private:
      //! \brief Helper method for applying an operation to a matrix
      template<typename... T, typename OP>
        inline auto apply_op(const OP& op, T&&... t) const
      {
        mat result;
        multirange_for([&](auto& r, auto&... m) { r = op(m...); }, result._col, _col, t...);
        return result;
      }

      //! \brief Helper method for applying an operation to a matrix in place
      template<typename... T, typename OP>
        inline auto& apply_op_inplace(const OP& op, T&&... t)
      {
        multirange_for([&](auto&... m) { op(m...); }, _col, t...);
        return *this;
      }
      

      /////////////////
      // Data Members
      /////////////////
      std::array<Vec, N> _col;
    };
    
  } // end detail namespace

  //! \brief create an identity matrix
  template<typename M>
    inline constexpr M identity(void) { return M(1); }

  //! \brief Bring 'transpose' in the bbm namespace
  template<typename M>
    inline constexpr M transpose(const M& m) { return transpose(m); }

  //! \brief 2D matrix
  template<typename T>
    using mat2d = bbm::detail::mat<vec2d<T>, 2>;

  //! \brief 3D matrix
  template<typename T>
    using mat3d = bbm::detail::mat<vec3d<T>, 3>;

} // end bbm namespace

#endif /* _BBM_MAT_H_ */
