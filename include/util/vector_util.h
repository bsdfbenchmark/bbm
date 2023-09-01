#ifndef _BBM_VECTOR_UTIL_H_
#define _BBM_VECTOR_UTIL_H_

#include <vector>
#include <ostream>
#include <type_traits>

#include "concepts/stringconvert.h"

#include "util/reference.h"
#include "util/type_traits.h"
#include "util/iterator_util.h"
#include "util/multirange_for.h"

/***********************************************************************/
/*! \file vector_util.h
    \brief Extensions for the STL vector class.
************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! @{ \name vector with support for references.
   *********************************************************************/ 
  template<typename T>
    class vector : public std::vector<T>
  {
    using base_type = std::vector<T>;
  public:
    using value_type = typename base_type::value_type;
    using allocator_type = typename base_type::allocator_type;
    using size_type = typename base_type::size_type;
    using difference_type = typename base_type::difference_type;
    using reference = typename base_type::reference;
    using const_reference = typename base_type::const_reference;
    using pointer = typename base_type::pointer;
    using const_pointer = typename base_type::const_pointer;
    using iterator = typename base_type::iterator;
    using const_iterator = typename base_type::const_iterator;
    using reverse_iterator = typename base_type::reverse_iterator;
    using const_reverse_iterator = typename base_type::const_reverse_iterator;

    using base_type::base_type;
    using base_type::operator=;
    using base_type::operator[];

    //! \brief Std vector Casting Constructor
    template<typename... D>
      vector(const std::vector<D...>& v) : base_type(v) {}
    
    //! \brief Casting Constructor
    template<typename U> requires concepts::assignable_to<U, T>
      vector(const bbm::vector<U>& src)
    {
      base_type::reserve(std::size(src));
      for(auto& s : src) base_type::push_back(s);
    }
    
    //! \brief Casting Assignemnt 
    template<typename U> requires concepts::assignable_to<U, T>
      bbm::vector<T>& operator=(const bbm::vector<U>& src)
    {
      vector temp(src);
      std::swap(*this, temp);
      return *this;
    }
    
    operator base_type() { return *this; }
    operator base_type() const { return *this; }
  };
  
  /*********************************************************************/
  /*! \brief vector<bbm::reference> with casting on the inspectors and iterators

    The problem with vector<bbm::reference> is that the result of any
    operation still needs to be manually cast when calling methods on
    the result.  This wrapper class will automatically do the casting.

    If not a reference, bbm::vector will be equivalent to a std::vector 
  **********************************************************************/
  template<typename T> requires std::is_reference_v<T>
  class vector<T> : public std::vector< bbm::reference<T> >
  {
    using base_type = std::vector<bbm::reference<T> >;
  public:
    using value_type = typename base_type::value_type;
    using allocator_type = typename base_type::allocator_type;
    using size_type = typename base_type::size_type;
    using difference_type = typename base_type::difference_type;
    using reference = T;
    using const_reference = bbm::add_const_t<T>;
    using pointer = typename base_type::pointer;
    using const_pointer = typename base_type::const_pointer;
    using iterator = cast_itr<T, typename base_type::iterator>;
    using const_iterator = cast_itr<bbm::add_const_t<T>, typename base_type::const_iterator>;
    using reverse_iterator = cast_itr<T, typename base_type::reverse_iterator>;
    using const_reverse_iterator = cast_itr<bbm::add_const_t<T>, typename base_type::const_reverse_iterator>;

    using base_type::base_type;
    using base_type::operator=;
    using base_type::size;

    //! \brief Casting Assignemnt 
    template<typename U> requires concepts::assignable_to<U, bbm::reference<T>>
      bbm::vector<T>& operator=(const bbm::vector<U>& src)
    {
      if (std::size(src) != size()) throw bbm_size_error;
      std::copy(std::begin(src), std::end(src), begin());
      return *this;
    }
    
    //! @{ \name Auto-casting dereference methods
    reference at(size_type idx) { return static_cast<reference>(base_type::at(idx)); }
    const_reference at(size_type idx) const { return static_cast<const_reference>(base_type::at(idx)); }

    reference operator[](size_type idx) { return static_cast<reference>(base_type::operator[](idx)); }
    const_reference operator[](size_type idx) const { return static_cast<const_reference>(base_type::operator[](idx)); }

    reference front(void) { return static_cast<reference>(base_type::front()); }
    const_reference front(void) const { return static_cast<const_reference>(base_type::front()); }

    reference back(void) { return static_cast<reference>(base_type::back()); }
    const_reference back(void) const { return static_cast<const_reference>(base_type::back()); }
    //! @}

    //! @{ \name Auto-casting iterators
    iterator begin(void) { return iterator(base_type::begin()); }
    const_iterator begin(void) const { return const_iterator(base_type::begin()); }
    iterator rbegin(void) { return iterator(base_type::rbegin()); }
    const_iterator rbegin(void) const { return const_iterator(base_type::rbegin()); }
    const_iterator cbegin(void) const {  return const_iterator(base_type::cbegin()); }

    iterator end(void) { return iterator(base_type::end()); }
    const_iterator end(void) const { return const_iterator(base_type::end()); }
    iterator rend(void) { return iterator(base_type::rend()); }
    const_iterator rend(void) const { return const_iterator(base_type::rend()); }
    const_iterator cend(void) const { return const_iterator(base_type::cend()); }
    //! @}

    //! \brief push_back reference encapsulation
    template<typename R>
      void push_back(R&& a) { base_type::push_back( bbm::reference<T>(std::forward<R>(a)) ); }

    //! @{ \name Override insert to use emplace
    constexpr iterator insert(const_iterator pos, value_type&& val) { return base_type::emplace(pos, std::forward<decltype(val)>(val)); }

    constexpr iterator insert(const_iterator pos, size_t count, value_type&& val)
    {
      size_t idx = std::distance(cbegin(), pos);
      
      // reserve memory
      base_type::reserve(base_type::size() + count);
      
      // compute emplace position
      auto epos = std::next(base_type::begin(), idx);
      
      // add
      for(size_t c=0; c != count; ++c)
        base_type::emplace(epos, std::forward<decltype(val)>(val));

      // Done.
      return std::prev(end(), count);
    }

    template<typename Itr>
      constexpr iterator insert(const_iterator pos, Itr inputBegin, Itr inputEnd)
    {
      size_t idx = std::distance(cbegin(), pos);
      
      // reserve memory
      base_type::reserve(base_type::size() + std::distance(inputBegin, inputEnd));

      // compute emplace position
      auto epos = std::next(base_type::begin(), idx);
      
      // add
      for(auto itr = inputBegin; itr != inputEnd; ++itr)
      {
        // special case: cast_itr wrapping around a bbm_reference
        if constexpr (is_bbm_reference_v<std::iter_value_t<underlying_itr_t<Itr>>>)
          base_type::emplace(epos, itr.raw_dereference());
        
        // regular iterator
        else base_type::emplace(epos, *itr);
      }
        
      // Done.
      return std::prev(end(), std::distance(inputBegin, inputEnd)); 
    }
    //! @}

    
  };


  /*** Implementation Details ***/
  namespace detail {

    template<typename T>
      struct is_bbm_vector_impl : std::false_type {};
    
    template<typename T>
      struct is_bbm_vector_impl<bbm::vector<T>> : std::true_type {};
  }

  /*********************************************************************/
  /*! @{ \name type traits for bbm::vector
   *********************************************************************/
  template<typename T>
    using is_bbm_vector = bbm::detail::is_bbm_vector_impl<std::decay_t<T>>;

  template<typename T>
    inline constexpr bool is_bbm_vector_v = is_bbm_vector<T>::value;
  //! @}

  
  /*** Implementation details for mat operators ***/
  namespace detail {

    template<typename OP, typename T, typename U> requires ((is_vector_v<T> || is_bbm_vector_v<T>) && !(is_vector_v<U> || is_bbm_vector_v<U>))
      inline auto vector_mathop(const T& vec, const U& u, const OP& op)
    {
      vector< decltype(vec[0] + u) > result;  result.reserve( vec.size() );
      for(auto& val : vec)
        result.push_back( op(val, u) );
      return result;
    }

    template<typename OP, typename T, typename U> requires ((is_vector_v<T> || is_bbm_vector_v<T>) && (is_vector_v<U> || is_bbm_vector_v<U>))
      inline auto vector_mathop(const T& t, const U& u, const OP& op)
    {
      vector< decltype(t[0] + u[0]) > result;  result.reserve( t.size() );
      multirange_for([&](auto& t, auto& u) { result.push_back( op(t, u) ); }, t, u);
      return result;
    }

    template<typename OP, typename T, typename U> requires ((is_vector_v<T> || is_bbm_vector_v<T>) && !(is_vector_v<U> || is_bbm_vector_v<U>) && !is_const_v<T>)
      inline auto& vector_mathop_inplace(T& vec, const U& u, const OP& op)
    {    
      for(auto& val : vec) op(val, u);
      return vec;
    }

    template<typename OP, typename T, typename U> requires ((is_vector_v<T> || is_bbm_vector_v<T>) && (is_vector_v<U> || is_bbm_vector_v<U>) && !is_const_v<T>)
      inline auto& vector_mathop_inplace(T& t, const U& u, const OP& op)
    {    
      multirange_for(op, t, u);
      return t;
    }

  }
  
  /*********************************************************************/
  /*! \@{ \name Math operators
   *********************************************************************/

  /*** Addition ***/
  template<typename T, typename U> inline auto& operator+=(vector<T>& v, const U& u) { return bbm::detail::vector_mathop_inplace(v, u, [](auto& v, auto& u) { v += u; } ); }
  template<typename T, typename U> inline auto operator+(const vector<T>& v, const U& u) { return bbm::detail::vector_mathop(v, u, [](auto& v, auto& u) { return (v + u); } ); }
  template<typename T, typename U> requires (!is_vector_v<T> && !is_bbm_vector_v<T>) inline auto operator+(const T& v, const vector<U>& u) { return (u + v); }

  /*** Subtraction ***/
  template<typename T, typename U> inline auto& operator-=(vector<T>& v, const U& u) { return bbm::detail::vector_mathop_inplace(v, u, [](auto& v, auto& u) { v -= u; } ); }
  template<typename T, typename U> inline auto operator-(const vector<T>& v, const U& u) { return bbm::detail::vector_mathop(v, u, [](auto& v, auto& u) { return (v - u); } ); }

  /*** Multiplication ***/
  template<typename T, typename U> inline auto& operator*=(vector<T>& v, const U& u) { return bbm::detail::vector_mathop_inplace(v, u, [](auto& v, auto& u) { v *= u; } ); }
  template<typename T, typename U> inline auto operator*(const vector<T>& v, const U& u) { return bbm::detail::vector_mathop(v, u, [](auto& v, auto& u) { return (v * u); } ); }
  template<typename T, typename U> requires (!is_vector_v<T> && !is_bbm_vector_v<T>) inline auto operator*(const T& v, const vector<U>& u) { return (u * v); }

  /*** Division ***/
  template<typename T, typename U> inline auto& operator/=(vector<T>& v, const U& u) { return bbm::detail::vector_mathop_inplace(v, u, [](auto& v, auto& u) { v /= u; } ); }
  template<typename T, typename U> inline auto operator/(const vector<T>& v, const U& u) { return bbm::detail::vector_mathop(v, u, [](auto& v, auto& u) { return (v / u); } ); }

  /*** Modulation ***/
  template<typename T, typename U> inline auto& operator%=(vector<T>& v, const U& u) { return bbm::detail::vector_mathop_inplace(v, u, [](auto& v, auto& u) { v %= u; } ); }
  template<typename T, typename U> inline auto operator%(const vector<T>& v, const U& u) { return bbm::detail::vector_mathop(v, u, [](auto& v, auto& u) { return (v % u); } ); }
  
  //! @}
  

  template<typename T>
    std::ostream& operator<<(std::ostream& s, const std::vector<T>& vec)
  {
    s << bbm::toString(vec);
    return s;
  }

} // end bbm namespace

#endif /* _BBM_VECTOR_UTIL_H_ */
