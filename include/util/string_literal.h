#ifndef _BBM_STRING_LITERAL_H_
#define _BBM_STRING_LITERAL_H_

#include <ostream>
#include <algorithm>

/***********************************************************************/
/*! \file string_litral.h
    \brief Minimal implementaion for string literals
************************************************************************/

namespace bbm {

  template<size_t N=1>
    struct string_literal
  {
    //! \brief Empty string
    inline constexpr string_literal(void)
    {
      value[0] = '\0';
    }
      
    //! \brief constructor
    inline constexpr string_literal(const char (&str)[N])
    {
      std::copy_n(str, N, value);
    }

    //! \brief Number of chars in the string_literal
    static constexpr size_t size = N;
    
    //! \brief true is empty (i.e., == '\0')
    static constexpr bool empty = (N==1);

    //! @{ \name iterator
    inline constexpr const char* begin(void) const { return value; }
    inline constexpr const char* end(void) const { return value + N - 1; }
    //! @}
    
    //! @{ \name compare two literals (constexpr)
    template<size_t M>
      inline constexpr bool operator==(string_literal<M> str) const
    {
      return (M == N) && std::equal(begin(), end(), str.value);
    }

    template<size_t M>
      inline constexpr bool operator==(const char (&str)[M]) const
    {
      return operator==(string_literal<M>(str));
    }

    template<size_t M>
      friend inline constexpr bool operator==(const char (&str)[M], string_literal<N> lit)
    {
      return (lit == string_literal<M>(str));
    }
    //! @}
    
    //! @{ \name concat two literals (constexpr)
    template<size_t M>
      inline constexpr string_literal<N+M-1> operator+(string_literal<M> str) const
    {
      char temp[N+M-1];
      std::copy_n(value, N-1, temp);
      std::copy_n(str.value, M, temp + N -1);
      return string_literal<N+M-1>(temp);
    }

    template<size_t M>
      inline constexpr auto operator+(const char (&str)[M]) const
    {
      return operator+(string_literal<M>(str));
    }

    template<size_t M>
      friend inline constexpr auto operator+(const char (&str)[M], string_literal<N> lit)
    {
      return string_literal<M>(str) + lit;
    }
    //! @}
    
    //! \brief get substring
    template<size_t S, size_t LEN=N-1> requires (S+LEN < N)
      inline constexpr auto substr(void) const
    {
      char temp[LEN+1];
      std::copy_n(begin()+S, LEN, temp);
      temp[LEN] = '\0';
      return string_literal<LEN+1>(temp);
    }

    //! \brief cast to C string
    inline constexpr operator const char*(void) const { return value; }
    
    //! \brief forward 'value' to stream
    friend std::ostream& operator<<(std::ostream& s, const string_literal<N>& str)
    {
      s << str.value;
      return s;
    }

    char value[N];
  };

  /**********************************************************************/
  /*! \brief custom literal operator for string_literals
   **********************************************************************/
  template<string_literal LIT>
    constexpr decltype(LIT) operator""_sl(void)
  {
    return LIT;
  }
  
  
  /*** Implementation details for is_string_literal_impl ***/
  namespace detail {

    template<typename T> struct is_string_literal_impl : std::false_type {};
    template<size_t N> struct is_string_literal_impl<string_literal<N>> : std::true_type {};
    
  } // end detail namespace
  
  /**********************************************************************/
  /*! @{ \name is_string_literal type trait
   **********************************************************************/
  template<typename T>
    using is_string_literal = bbm::detail::is_string_literal_impl<std::decay_t<T>>;

  template<typename T>
    inline constexpr bool is_string_literal_v = is_string_literal<T>::value;
  
} // end bbm namespace
  
#endif /* _BBM_STRING_LITERAL_H_ */
