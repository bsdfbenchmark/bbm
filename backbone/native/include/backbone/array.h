#ifndef _BBM_NATIVE_ARRAY_H_
#define _BBM_NATIVE_ARRAY_H_

#include <array>
#include <ostream>
#include <concepts>
#include <functional>
#include "util/multirange_for.h"
#include "util/reference.h"

/************************************************************************/
/*! \file array.h

  \brief Core data structure for color and vecNd.

*************************************************************************/

namespace backbone {

  // Forward Declaration
  template<typename T, size_t N> struct array;
  
  /*** Implementation details for array ***/
  namespace detail {

    template<typename T, size_t N>
      struct array_impl : public std::array<T,N>
    {
      using base_type = std::array<T,N>;
    public:
      
      //! @{ \name Inherited methods from std::array
      using base_type::operator[];
      using base_type::begin;
      using base_type::rbegin;
      using base_type::end;
      using base_type::rend;

      using base_type::empty;
      using base_type::size;
      using typename base_type::value_type;
      //! @}

      //! \brief Copy constructor
      template<typename U> requires std::convertible_to<U,T>
        array_impl(const array_impl<U,N>& src) : base_type() { std::copy(src.begin(), src.end(), begin()); }

      //! \brief Construct an array and set every element to 't'
      array_impl(const T& t=0) : base_type() { std::fill(begin(), end(), t); }
          
      //! \brief Construct an array; use std::array constructor.
      template<typename... U> requires (sizeof...(U) > 1)
        array_impl(U&&... u) : base_type{T(u)...} {}

      //! \brief Construct from an std::array
      array_impl(std::array<T,N>&& src) : base_type(std::forward<decltype(src)>(src)) {}
      
      //! \brief assignment operator of array
      template<typename U> requires requires(T a, U b) {{a=b};}
        array_impl& operator=(const array_impl<U,N>& src)
      {
        std::copy(src.begin(), src.end(), begin());
        return *this;
      }

      //! \brief assignment operator of value
      template<typename U> requires requires(T a, U b) {{a=b};}
        array_impl& operator=(U&& src)
      {
        std::fill(begin(), end(), src);
        return *this;
      }
      
      //! @{ \name Basic Math Operators
      inline auto operator-(void) const { return apply_op( std::negate<>() ); }
      inline auto& operator++(void) { return apply_op_inplace( [](auto& r) { return ++r; }); }
      inline auto& operator--(void) { return apply_op_inplace( [](auto& r) { return --r; }); }
      inline auto operator++(int) { auto ret = *this; ++(*this); return ret; }
      inline auto operator--(int) { auto ret = *this; ++(*this); return ret; }
      
      template<typename U> inline constexpr auto operator+(U&& u) const { return apply_op( std::plus<>(), u); }
      template<typename U> inline constexpr auto operator-(U&& u) const { return apply_op( std::minus<>(), u); }
      template<typename U> inline constexpr auto operator*(U&& u) const { return apply_op( std::multiplies<>(), u); }
      template<typename U> inline constexpr auto operator/(U&& u) const { return apply_op( std::divides<>(), u); }
      template<typename U> inline constexpr auto operator%(U&& u) const { return apply_op( std::modulus<>(), u); }

      template<typename U> inline constexpr auto& operator+=(U&& u) { return apply_op_inplace( [](auto& r, auto& v) { r += v; }, u); }
      template<typename U> inline constexpr auto& operator-=(U&& u) { return apply_op_inplace( [](auto& r, auto& v) { r -= v; }, u); }
      template<typename U> inline constexpr auto& operator*=(U&& u) { return apply_op_inplace( [](auto& r, auto& v) { r *= v; }, u); }
      template<typename U> inline constexpr auto& operator/=(U&& u) { return apply_op_inplace( [](auto& r, auto& v) { r /= v; }, u); }
      template<typename U> inline constexpr auto& operator%=(U&& u) { return apply_op_inplace( [](auto& r, auto& v) { r %= v; }, u); }

      friend inline constexpr auto operator+(const T& t, const array_impl& a) { return (a + t); }
      friend inline constexpr auto operator-(const T& t, const array_impl& a) { return (array<T,N>(t) - a); }
      friend inline constexpr auto operator*(const T& t, const array_impl& a) { return (a * t); }
      friend inline constexpr auto operator/(const T& t, const array_impl& a) { return (array<T,N>(t) / a); }
      friend inline auto operator%(const T& t, const array_impl& a) { return (array<T,N>(t) % a); }
      // @}

      //! @{ name Bit Operators
      template<typename U> inline constexpr auto operator&(U&& u) const { return apply_op( std::bit_and<>(), u); }
      template<typename U> inline constexpr auto operator|(U&& u) const { return apply_op( std::bit_or<>(), u); }
      template<typename U> inline constexpr auto operator^(U&& u) const { return apply_op( std::bit_xor<>(), u); }
      inline constexpr auto operator~(void) const { return apply_op( std::bit_not<>()); }
    
      template<typename U> inline constexpr auto operator&=(U&& u) { return apply_op_inplace( [](auto& r, auto& v) { r &= v; }, u); }
      template<typename U> inline constexpr auto operator|=(U&& u) { return apply_op_inplace( [](auto& r, auto& v) { r |= v; }, u); }
      template<typename U> inline constexpr auto operator^=(U&& u) { return apply_op_inplace( [](auto& r, auto& v) { r ^= v; }, u); }

      friend inline constexpr auto operator|(const T& t, const array_impl& a) { return (a | t); }
      friend inline constexpr auto operator&(const T& t, const array_impl& a) { return (a & t); }
      friend inline constexpr auto operator^(const T& t, const array_impl& a) { return (a ^ t); }
      //! @}
      
      //! @{ \name Logical Operators
      template<typename U> inline constexpr auto operator&&(U&& u) const { return apply_op( std::logical_and<>(), u); }
      template<typename U> inline constexpr auto operator||(U&& u) const { return apply_op( std::logical_or<>(), u); }
      inline constexpr auto operator!(void) const { return apply_op( std::logical_not<>()); }

      friend inline constexpr auto operator&&(const T& t, const array_impl& a) { return (a && t); }
      friend inline constexpr auto operator||(const T& t, const array_impl& a) { return (a || t); }
      //! @}
      
      //! @{ \name Comparison Operators
      template<typename U> inline constexpr auto operator<(U&& u) const { return apply_op( std::less<>(), u); }
      template<typename U> inline constexpr auto operator<=(U&& u) const { return apply_op( std::less_equal<>(), u); }
      template<typename U> inline constexpr auto operator>(U&& u) const { return apply_op( std::greater<>(), u); }
      template<typename U> inline constexpr auto operator>=(U&& u) const { return apply_op( std::greater_equal<>(), u); }

      friend inline constexpr auto operator<(const T& t, const array_impl& a) { return (a > t); }
      friend inline constexpr auto operator<=(const T& t, const array_impl& a) { return (a >= t); }
      friend inline constexpr auto operator>(const T& t, const array_impl& a) { return (a < t); }
      friend inline constexpr auto operator>=(const T& t, const array_impl& a) { return (a <= t); }
      
      template<typename U> friend inline constexpr bool operator==(const array_impl& t, const array_impl<U,N>& u) { auto res = t.apply_op( std::equal_to<>(), u); return std::all_of(res.begin(), res.end(), std::identity()); }
      template<typename U> friend inline constexpr bool operator!=(const array_impl& t, const array_impl<U,N>& u) { auto res = t.apply_op( std::equal_to<>(), u); return std::none_of(res.begin(), res.end(), std::identity()); }
      //! @}
      
      //! \brief Print to std::ostream
      friend std::ostream& operator<<(std::ostream& s, const array_impl<T,N>& a)
      {
        s << "[";
        for(auto& v : a)
        {
          if(&v != a.begin())  s << ", ";
          s << v;
        }
        s << "]";
        return s;
      }
      
    private:
      //! \brief Helper method for applying an operator returning a new array.
      template<typename... U, typename OP>
        inline auto apply_op(const OP& op, U&&... u) const
      {
        array< decltype( op( *begin(), (*bbm::begin(u))... ) ), N> result;
        bbm::multirange_for([&](auto& result, auto&... arg) { result = op(arg...); }, result, *this, u...);
        return result;
      }

      //! \brief Helper method for applying an operator in place
      template<typename... U, typename OP>
        inline auto& apply_op_inplace(const OP& op, U&&... u)
      {
        bbm::multirange_for([&](auto&... arg) { op(arg...); }, *this, u...);
        return *this;
      }
    };
  } // detail namespace

  
  /**********************************************************************/
  /*! @{ \name array type
   **********************************************************************/
  template<typename T, size_t N> requires (!std::is_reference_v<T>)
    struct array<T, N> : public detail::array_impl<T, N>
  {
    using base_type = detail::array_impl<T,N>;
    using base_type::base_type;
    using base_type::operator=;
  };

  template<typename T, size_t N> requires (std::is_reference_v<T>)
    struct array<T, N> : public detail::array_impl<bbm::reference<T>, N>
  {
    using base_type = detail::array_impl<bbm::reference<T>,N>;
    using base_type::base_type;
    using base_type::operator=;
  };
  //! @}

  /*** Implementation details for is_array ***/
  namespace detail {
    
    template<typename T> struct is_array : std::false_type {};
    template<typename T, size_t N>  struct is_array<array<T,N>> : std::true_type {};
  } // end detail namespace
  
  /********************************************************************/
  /*! @{ \name is_array type traits
   ********************************************************************/
  template<typename T>
  using is_array = detail::is_array<std::decay_t<T>>;

  template<typename T>
    inline constexpr bool is_array_v = is_array<T>::value;
  //! @}
  
  /*** Implementation details for array_size ***/
  namespace detail {
    template<typename T> struct array_size { static constexpr size_t value = 1; };
    template<typename T, size_t N> struct array_size<array<T,N>> { static constexpr size_t value = N; };
  } // end detail namespace

  
  /********************************************************************/
  /*! \brief array_size type traits
   ********************************************************************/
  template<typename T> 
    static constexpr size_t array_size = detail::array_size< std::decay_t<T> >::value;
  
} // end backbone namespace

#endif /* _BBM_NATIVE_ARRAY_H_ */
