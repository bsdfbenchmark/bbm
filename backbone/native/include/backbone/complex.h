#ifndef _BBM_NATIVE_COMPLEX_H_
#define _BBM_NATIVE_COMPLEX_H_

#include <complex>
#include "backbone/array.h"
#include "backbone/vec.h"

/************************************************************************/
/*! \filename complex.h

  \brief Complex numbers; built on top of array

************************************************************************/

namespace backbone {

  /**********************************************************************/
  /*! \brief Complex numbers
   **********************************************************************/
  template<typename T>
    struct complex : public array<T, 2>
  {
    using base_type = array<T,2>;
    using base_type::operator=;
    
    //! @{ \name Constructors
    template<typename U=T> requires std::constructible_from<T,U>
      constexpr complex(const T& r=0, const U& i=0) : base_type(r, i) {}

    template<typename U> requires std::constructible_from<T,U>
      constexpr complex(const array<U,2>& src) : base_type(src) {}
    
    template<typename U> requires std::constructible_from<T, U>
      constexpr complex(const std::complex<U>& src) : base_type( src.real(), src.imag() ) {}
    //! @}

    //! @{ \name Conversion to std::complex (with optimal float or double type based on T) -- not on 'U'!
    inline constexpr auto std_complex(void) const -> decltype(std::complex(std::declval<T>(), std::declval<T>()))
    {
      return {real(*this), imag(*this)};
    }
            
    template<typename U>
      static inline constexpr auto std_complex(complex<U> u) -> decltype(std::complex(std::declval<T>(), std::declval<T>()))
    {
      using type = typename decltype(std_complex())::value_type;
      return {type(real(u)), type(imag(u))};
    }
    //! @}

    //! \brief Specialized assignment operator
    template<typename U> requires std::constructible_from<T, U>
      complex& operator=(U&& r) { *this = complex(std::forward<U>(r),0); return *this; }

    
    //! @{ \name Addition
    inline constexpr complex operator+(T u) const { return *this + complex(u,0); }
    template<typename U> inline constexpr complex operator+(complex<U> u) const { return std_complex() + complex<T>::std_complex(u); }
    friend constexpr inline complex operator+(T u, complex& t) { return complex(u) + t; }

    template<typename U> inline constexpr complex& operator+=(U u) { *this = *this + u; return *this; }
    //! @}

    //! @{ \name Subtraction
    inline constexpr complex operator-(T u) const { return *this - complex(u,0); }
    template<typename U> inline constexpr complex operator-(complex<U> u) const { return std_complex() + complex<T>::std_complex(u); }
    friend constexpr inline complex operator-(T u, complex& t) { return complex(u) - t; }

    template<typename U> inline constexpr complex& operator-=(U u) { *this = *this - u; return *this; }
    //! @}

    //! @{ \name Multiplication
    inline constexpr complex operator*(T u) const { return std_complex() * u; }
    template<typename U> inline constexpr complex operator*(complex<U> u) const { return std_complex() * complex<T>::std_complex(u); }
    friend constexpr inline complex operator*(T u, complex& t) { return complex(u) * t; }

    template<typename U> inline constexpr complex& operator*=(U u) { *this = *this * u; return *this; }
    //! @}
    
    //! @{ \name Division
    inline constexpr complex operator/(T u) const { return std_complex() / u; }
    template<typename U> inline constexpr complex operator/(complex<U> u) const { return std_complex() / complex<T>::std_complex(u); }
    friend constexpr inline complex operator/(T u, complex& t) { return complex<T>::std_complex(u) / complex<T>::std_complex(t); }
    template<typename U> inline constexpr complex& operator/=(U u) { *this = *this / u; return *this; }
    //! @}
  };
  
  //! @{ \name Core Inspectors
  template<typename T> inline constexpr T real(complex<T> z) { return z[0]; }
  template<typename T> inline constexpr T imag(complex<T> z) { return z[1]; }
  template<typename T> inline constexpr complex<T> conj(complex<T> z) { return std::conj(z.std_complex()); }
  //! @}

  //! \brief rcp
  template<typename T> inline constexpr complex<T> rcp(complex<T> z)
  {
    auto l = squared_norm(z);
    return conj(z) / l;
  }

  //! @{ \name std::complex -> complex Functions
  #define BBM_DECL_COMPLEX_OP(op) template<typename T> inline constexpr complex<T> op (complex<T> z) { return std::op (z.std_complex()); }
    
  BBM_DECL_COMPLEX_OP( exp );
  BBM_DECL_COMPLEX_OP( log );
  BBM_DECL_COMPLEX_OP( pow );
  BBM_DECL_COMPLEX_OP( sqrt );
  BBM_DECL_COMPLEX_OP( sin );
  BBM_DECL_COMPLEX_OP( cos );
  BBM_DECL_COMPLEX_OP( tan );
  BBM_DECL_COMPLEX_OP( asin );
  BBM_DECL_COMPLEX_OP( acos );
  BBM_DECL_COMPLEX_OP( atan );
  BBM_DECL_COMPLEX_OP( sinh );
  BBM_DECL_COMPLEX_OP( cosh );
  BBM_DECL_COMPLEX_OP( tanh );
  BBM_DECL_COMPLEX_OP( asinh );
  BBM_DECL_COMPLEX_OP( acosh );
  BBM_DECL_COMPLEX_OP( atanh );
  
  #undef BBM_DECL_COMPLEX_OP
  //! @}

  //! \brief cossin method
  template<typename T> inline constexpr vec2d<complex<T>> cossin(complex<T> z) { return vec2d<complex<T>>( cos(z), sin(z) ); }

  //! @{ \name Horizontal methods
  template<typename T> inline constexpr complex<T> reverse(complex<T> z) { return reverse(typename complex<T>::base_type(z)); }
  template<typename T> inline constexpr complex<T> psum(complex<T> z) { return psum(typename complex<T>::base_type(z)); }
  template<typename T> inline constexpr complex<T> normalize(complex<T> z) { return normalize(typename complex<T>::base_type(z)); }
  //! @}

  /*** Implementation Details for is_complex_impl ***/
  namespace detail {

    template<typename T> struct is_complex_impl : std::false_type {};
    template<typename T> struct is_complex_impl<complex<T>> : std::true_type {};
    
  } // end detail namespace


  /*** Specializations for is_array and array_size ***/
  namespace detail {
    template<typename T> struct is_array<complex<T>> : std::true_type {};
    template<typename T> struct array_size<complex<T>> { static constexpr size_t value = 2; };
  }
  
  /**********************************************************************/
  /*! @{ \name type traits
   **********************************************************************/
  template<typename T>
    using is_complex = detail::is_complex_impl<std::decay_t<T>>;

  template<typename T>
    inline constexpr bool is_complex_v = is_complex<T>::value;
  //! @}

} // end backbone namespace


#endif /* _BBM_NATIVE_COMPLEX_H_ */
