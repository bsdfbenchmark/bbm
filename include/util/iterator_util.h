#ifndef _BBM_ITERATOR_UTIL_H_
#define _BBM_ITERATOR_UTIL_H_

#include <memory>
#include <utility>
#include <iterator>
#include <ranges>

#include "concepts/util.h"

/***********************************************************************/
/*! \file iterator_util.h
  \brief Extensions for STL iterators/ranges.
************************************************************************/

namespace bbm {
  
  /*********************************************************************/
  /*! @{ \name Extensions of size, begin, and end to non-iterable types
  **********************************************************************/
  template<typename T>
    size_t size(T&& t)
  {
    if constexpr (std::ranges::range<T>) return std::ranges::size(t);
    else return 1;
  }

  template<typename T>
    auto begin(T&& t)
  {
    if constexpr (std::ranges::range<T>) return std::ranges::begin(t);
    else return std::addressof(t);
  }
  
  template<typename T> 
    auto cbegin(T&& t)
  {
    if constexpr (std::ranges::range<T>) return std::ranges::cbegin(t);
    else  return bbm::begin(std::as_const(t));
  }
  
  template<typename T> 
    auto end(T&& t)
  {
    if constexpr (std::ranges::range<T>) return std::ranges::end(t);
    else return std::next(bbm::begin(t));
  }

  template<typename T> 
    auto cend(T&& t)
  {
    if constexpr (std::ranges::range<T>) return std::ranges::cend(t);
    else return bbm::end(std::as_const(t));
  }
  //! @}

  /*********************************************************************/
  /*! @{ \name Iterable container type traits
   *********************************************************************/
  template<typename T> requires std::ranges::range<T>
    using iterable_value_t = std::decay_t<decltype( *bbm::begin(std::declval<T>()) )>;
  //! @}
  
  /*********************************************************************/
  /*! \brief Cast iterator; cast the result after deferencing.
  **********************************************************************/
  template<typename CASTTYPE, typename ITR> requires std::input_or_output_iterator<ITR>
  class cast_itr
  {
  public:
    using iterator_category = typename std::iterator_traits<ITR>::iterator_category;
    using difference_type = typename std::iterator_traits<ITR>::difference_type;
    using value_type = std::remove_reference_t<CASTTYPE>;
    using reference = value_type&;
    using pointer = value_type*;

    //! \brief empty constructor
    cast_itr(void) : _itr() {}
    
    //! \brief constructor
    cast_itr(const ITR& itr) : _itr(itr) {}

    //! \brief construct from cast_itr
    template<typename C, typename I>
      cast_itr(const cast_itr<C,I>& itr) : _itr(itr.raw_iterator()) {}
    
    //! @{ \name comparison
    bool operator==(const cast_itr& itr) const { return (_itr == itr._itr); }
    bool operator!=(const cast_itr& itr) const { return (_itr != itr._itr); }
    bool operator<=(const cast_itr& itr) const { return (_itr <= itr._itr); }
    bool operator<(const cast_itr& itr) const { return (_itr < itr._itr); }
    bool operator>=(const cast_itr& itr) const { return (_itr >= itr._itr); }
    bool operator>(const cast_itr& itr) const { return (_itr > itr._itr); }
    //! @}

    //! @{ \name math operations
    cast_itr& operator++(void) { ++_itr; return *this; }
    cast_itr operator++(int) { cast_itr ret = *this; ++(*this); return ret; }
    cast_itr& operator+=(size_t step) { _itr += step; return *this; }
    cast_itr operator+(size_t step) const { return cast_itr(_itr + step); }

    cast_itr& operator--(void) { --_itr; return *this; }
    cast_itr operator--(int) { cast_itr ret = *this; --(*this); return ret; }
    cast_itr& operator-=(size_t step) { _itr -= step; return *this; }
    cast_itr operator-(size_t step) const { return cast_itr(_itr - step); }

    difference_type operator-(const cast_itr& itr) { return _itr - itr._itr; }

    auto operator[](size_t idx) const { return *(*this + idx); }
    //! @}

    //! \brief cast the derefenced value to CASTYPE&
    reference operator*(void) const { return static_cast<reference>(*_itr); }

    //! \brief cast the dereferenced value to CASTTYPE*
    pointer operator->(void) const { return &(this->operator*()); }

    //! \brief dereference without casting
    decltype(auto) raw_dereference(void) const { return *_itr; }

    //! \brief underlying iterator
    auto raw_iterator(void) const { return _itr; }
    
  private:
    ITR _itr;
  };

  /*** Implementation Details  ***/
  namespace detail {
    template<typename T> struct is_cast_itr_impl : std::false_type {};
    template<typename C, typename T> struct is_cast_itr_impl<cast_itr<C, T>> : std::true_type{};

    template<typename T> struct underlying_itr_impl { using type = T; };
    template<typename C, typename T> struct underlying_itr_impl<cast_itr<C, T>> { using type = T; };
  } // end detail namespace
  
  /*********************************************************************/
  /*! @{ Type traits
   *********************************************************************/
  template<typename T>
    using is_cast_itr = bbm::detail::is_cast_itr_impl<std::decay_t<T>>;

  template<typename T>
    inline constexpr bool is_cast_itr_v = is_cast_itr<T>::value;

  template<typename T>
    using underlying_itr_t = typename bbm::detail::underlying_itr_impl<std::decay_t<T>>::type;
  //! @}
  
} // end bbm namespace

#endif /* _BBM_ITERATOR_UTIL_H_ */
