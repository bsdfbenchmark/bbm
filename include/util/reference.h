#ifndef _BBM_REFERENCE_H_
#define _BBM_REFERENCE_H_

#include <ostream>

#include "core/error.h"
#include "util/type_traits.h"
#include "util/pointer.h"


/***********************************************************************/
/*! \file reference.h
    \brief Assignable reference with wrapper support for rvalues
************************************************************************/

namespace bbm {

  /*** Implementation details for reference_impl ***/
  namespace detail {

    template<typename T, typename PTR=std::add_pointer_t<std::remove_reference_t<T>>>
      struct reference_impl
    {
      //! @{ \name constructors
      inline constexpr reference_impl(std::nullptr_t = nullptr) noexcept : _ptr(nullptr) {}
      inline constexpr reference_impl(std::add_lvalue_reference_t<T> ref) noexcept : _ptr(std::addressof(ref)) {}
      inline constexpr reference_impl(const reference_impl& ref) noexcept = default;
      //! @}

      //! @{ \name assignment
      inline constexpr reference_impl& operator=(const reference_impl& ref) noexcept = default;
    
      template<typename R>
        inline constexpr reference_impl& operator=(const R& val)
      {
        static_assert(!bbm::is_const_v<T>, BBM_CONST_ASSIGNMENT);
        std::add_lvalue_reference_t<T>(*this) = val;
        return *this;
      }
      //! @}

      //! @{ \name cast operators
      inline constexpr operator std::add_lvalue_reference_t<T>() const
      {
        if(!is_dereferenceable()) throw bbm_unassigned_ref;
        return *_ptr;
      }
      
      template<typename TYPE> requires std::constructible_from<TYPE, T>
        inline constexpr operator TYPE(void) const
      {
        if(!is_dereferenceable()) throw bbm_unassigned_ref;
        return TYPE(*_ptr);
      }
      //! @}
      
      //! \brief forward operator
      inline constexpr PTR operator->(void) const { return _ptr; }

      //! \brief check if dereferenceable
      inline constexpr bool is_dereferenceable(void) const { return (_ptr != nullptr); }

      //! \brief assign a new reference
      template<typename R>
        inline constexpr void reassign(R&& ref) noexcept { *this = reference_impl<T, PTR>(std::forward<R>(ref)); }

      //! \brief ostream support
      friend std::ostream& operator<<(std::ostream& s, const reference_impl<T,PTR>& ref)
      {
        if(ref.is_dereferenceable()) s << T(ref);
        else s << "none";
        return s;
      }
      
    protected:
      PTR _ptr;
    };

  } // end detail namespace

  /**********************************************************************/
  /*! \brief Non-persistent reference (i.e., cannot take rvalues)
    *********************************************************************/
  template<typename T>
    struct reference : public bbm::detail::reference_impl<T, std::add_pointer_t<std::remove_reference_t<T>>>
  {
    using base_type = bbm::detail::reference_impl<T, std::add_pointer_t<std::remove_reference_t<T>>>;
    using base_type::base_type;
    using base_type::operator=;

    //! \brief Disallow rvalue construction
    inline constexpr reference(std::decay_t<T>&& ) = delete;
  };

  //! \brief const non-persistent reference
  template<typename T>
    using const_reference = reference<bbm::add_const_t<T>>;
  
  //! \brief Declaration of persistent_reference specialized below.
  template<typename T> struct persistent_reference;
  
  /**********************************************************************/
  /*! \brief Persistent reference (no difference for non const references)
   **********************************************************************/
  template<typename T> requires (!bbm::is_const_v<T>)
    struct persistent_reference<T> : public reference<T>
  {
    using reference<T>::reference;
    using reference<T>::operator=;
  };

  /**********************************************************************/
  /*! \brief Persistent reference (extend lifetime of const rvalue references)
   **********************************************************************/
  template<typename T> requires bbm::is_const_v<T>
    struct persistent_reference<T> : public bbm::detail::reference_impl<T, bbm::pointer<T>>
  {
    using base_type = bbm::detail::reference_impl<T, bbm::pointer<T>>;
    using base_type::base_type;
    using base_type::operator=;

    //! \brief Handle assignment to a const reference of a temporary variable => make_pointer
    //! This occurs if:
    //! 1) R != persistent_reference<T>     (use copy constructor in this case)
    //! 2) if R == rvalue itself and R==T
    //!    else if an implicit conversion results in an rvalue (i.e., R is implicitely convertible to T and R!=T).
    //! This explicitely avoids (non-rvalue) T
    template<typename R> requires (!std::is_same_v<persistent_reference<T>, std::decay_t<R>>) &&
                                  ((std::is_rvalue_reference_v<R&&> && std::same_as<std::remove_cvref_t<R>, std::remove_cvref_t<T>>) ||
                                   (std::constructible_from<std::remove_reference_t<T>, R> && !std::same_as<std::remove_cvref_t<R>, std::remove_cvref_t<T>>))
      inline constexpr persistent_reference(R&& val) noexcept
    {
      base_type::_ptr = std::make_shared<std::remove_reference_t<T>>(std::forward<R>(val));
    }
  };
  
  //! \brief const_persistent_reference
  template<typename T>
    using const_persistent_reference = persistent_reference<bbm::add_const_t<T>>;

  
  /*** Implementation Details for is_bbm_reference ***/
  namespace detail {
    template<typename T> struct is_bbm_reference_impl : std::false_type {};
    template<typename T, typename PTR> struct is_bbm_reference_impl<reference_impl<T,PTR>> : std::true_type {};
  } // end detail namespace
    
  /*********************************************************************/
  /*! \@{ \name type traits
   *********************************************************************/
  template<typename T>
    using is_bbm_reference = bbm::detail::is_bbm_reference_impl<std::decay_t<T>>;

  template<typename T>
    inline constexpr bool is_bbm_reference_v = is_bbm_reference<T>::value;
  //! @}

} // end bbm namespace


#endif /* _BBM_REFERENCE_H_ */
