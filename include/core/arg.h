#ifndef _BBM_ARG_H_
#define _BBM_ARG_H_

#include <ostream>
#include "util/toString.h"
#include "util/reference.h"
#include "util/typestring.h"
#include "util/string_literal.h"


/************************************************************************/
/*! \file arg.h
  \brief Structure to store an argument, with possibly a type, name and default value.
  Refer to args.h for its usage.
*************************************************************************/


namespace bbm {

  /**********************************************************************/
  /*! \brief Forward declaration of bbm::arg

    \tparam Type = type of the argument
    \tparam Name = argument name
    \tparam Default = invocable type that returns the default value
  ***********************************************************************/
  template<typename Type, string_literal Name, typename Default=void> struct arg;

  
  /**********************************************************************/
  /*! @{ \name type traits
   **********************************************************************/
  namespace detail {
    template<typename T> struct is_arg_impl : std::false_type {};
    template<typename T, string_literal N, typename D> struct is_arg_impl<arg<T,N,D>> : std::true_type {};
  }

  template<typename T>
    using is_arg = bbm::detail::is_arg_impl<std::decay_t<T>>;

  template<typename T>
    inline constexpr bool is_arg_v = is_arg<T>::value;
  //! @}

  
  /**********************************************************************/
  /*! \brief Untyped Argument (only a name)

    The assignment operator is overriden to return a typed argument with the
    same name but with a type assigned.
  ***********************************************************************/
  template<string_literal Name>
    struct arg<void, Name, void>
  {
    static constexpr string_literal name = Name;
    using type = void;
    
    //! \brief Assignment of non-rvalue
    template<typename Type> requires (!std::is_rvalue_reference_v<Type&&>)
      inline constexpr decltype(auto) operator=(Type&& val)
    {
      return arg<Type, Name, void>(std::forward<Type>(val));
    }

    //! \brief Assignment of rvalue
    template<typename Type> requires std::is_rvalue_reference_v<Type&&>
      inline constexpr decltype(auto) operator=(Type&& val)
    {
      using rtype = std::add_lvalue_reference_t<std::add_const_t<Type>>;
      return arg<rtype, Name, void>(std::forward<Type>(val));
    }
  };

  
  /**********************************************************************/
  /*! brief Non-reference Typed Argument without Default Value
   **********************************************************************/
  template<typename Type, string_literal Name> requires (!std::is_void_v<Type> && !std::is_reference_v<Type>)
    struct arg<Type, Name, void>
  {
    template<typename T, string_literal N, typename D> friend struct arg;
    static constexpr string_literal name = Name;
    using type = Type;

    //! @{ \name Constructors
    inline constexpr arg(void) = delete;
    inline constexpr arg(const std::decay_t<type>& val) : _value(val) {};

    template<typename OtherType, string_literal OtherName, typename OtherDefault> requires std::constructible_from<std::decay_t<type>, OtherType>
      inline constexpr arg(const arg<OtherType, OtherName, OtherDefault>& src) : arg(src._value) {}
    //! @}

    //! \brief Assignment
    inline constexpr arg& operator=(const std::decay_t<type>& val) { _value = val; return *this; }

    //! @{ \name Return the stored value
    inline constexpr std::add_const_t<type>& value(void) const { return _value; }
    inline constexpr type& value(void) { return _value; }
    //! @}
      
  private:
    std::decay_t<type> _value;
  };

  
  /**********************************************************************/
  /*! brief Non-reference Typed Argument with Default Value
   **********************************************************************/
  template<typename Type, string_literal Name, typename Default> requires (!std::is_void_v<Type> && !std::is_reference_v<Type>) && std::invocable<Default>
    struct arg<Type, Name, Default>
  {
    template<typename T, string_literal N, typename D> friend struct arg;
    static constexpr string_literal name = Name;
    using type = Type;

    //! @{ \name Constructors
    inline constexpr arg(void) : _value( Default()() ) {};
    inline constexpr arg(const std::decay_t<type>& val) : _value(val) {}

    template<typename OtherType, string_literal OtherName, typename OtherDefault>  requires std::constructible_from<std::decay_t<type>, OtherType>
      inline constexpr arg(const arg<OtherType, OtherName, OtherDefault>& src) : arg(src._value) {}
    //! @}

    //! \brief Assignment
    inline constexpr arg& operator=(const std::decay_t<type>& val) { _value = val; return *this; }

    //! @{ \name Return the stored value
    inline constexpr std::add_const_t<type>& value(void) const { return _value; }
    inline constexpr type& value(void) { return _value; }
    //! @}
      
  private:
    std::decay_t<type> _value;
  };

  
  /**********************************************************************/
  /*! brief Reference Typed Argument without Default Value

    Assignment operator reassign the reference, not the value!
   **********************************************************************/
  template<typename Type, string_literal Name> requires (!std::is_void_v<Type> && std::is_reference_v<Type>)
    struct arg<Type, Name, void>
  {
    template<typename T, string_literal N, typename D> friend struct arg;
    static constexpr string_literal name = Name;
    using type = Type;

    //! @{ \name Constructors
    inline constexpr arg(void) = delete;
    template<typename OtherType, string_literal OtherName, typename OtherDefault>  requires std::constructible_from<bbm::persistent_reference<type>, OtherType>
      inline constexpr arg(const arg<OtherType, OtherName, OtherDefault>& src) : _value(src._value) {}

    template<typename T> requires (!is_arg_v<T> && std::constructible_from<bbm::persistent_reference<type>, T&&>)
      inline constexpr arg(T&& val) : _value(std::forward<T>(val)) {};
    //! @}

    //! \brief Assignment
    template<typename T>
      inline constexpr arg& operator=(T&& src)
    {
      _value.reassign( std::forward<T>(src) );
      return *this;
    }
    
    //! @{ \name Return the stored value
    inline constexpr type value(void) const { return _value; }
    inline constexpr type value(void) { return _value; }
    //! @}
      
  private:
    bbm::persistent_reference<type> _value;
  };

  
  /**********************************************************************/
  /*! brief Reference Typed Argument with Default Value

    Assignment operator reassign the reference, not the value!
   **********************************************************************/
  template<typename Type, string_literal Name, typename Default> requires (!std::is_void_v<Type> && std::is_reference_v<Type>) && std::invocable<Default>
    struct arg<Type, Name, Default>
  {
    template<typename T, string_literal N, typename D> friend struct arg;
    static constexpr string_literal name = Name;
    using type = Type;

    //! @{ \name Constructors
    inline constexpr arg(void) : _value( Default()() ) {}

    template<typename OtherType, string_literal OtherName, typename OtherDefault>  requires std::constructible_from<bbm::persistent_reference<type>, OtherType>
    inline constexpr arg(const arg<OtherType, OtherName, OtherDefault>& a) : _value(a._value) {}

    template<typename T> requires (!is_arg_v<T> && std::constructible_from<bbm::persistent_reference<type>, T&&>)
    inline constexpr arg(T&& val) : _value(std::forward<T>(val)) {}
    //! @}

    //! \brief Assignment
    template<typename T> 
      inline constexpr arg& operator=(T&& src)
    {
      _value.reassign( std::forward<T>(src) );
      return *this;
    }

    //! @{ \name Return the stored value
    inline constexpr type value(void) const { return _value; }
    inline constexpr type value(void) { return _value; }
    //! @}
      
  private:
    bbm::persistent_reference<type> _value;
  };


  /**********************************************************************/
  /*! \brief Print bbm::arg
   **********************************************************************/
  template<typename T, bbm::string_literal N, typename D>
    std::ostream& operator<<(std::ostream& s, const bbm::arg<T,N,D>& arg)
  {
    // print type
    if constexpr (!std::is_void_v<T>)
    {
      s << bbm::typestring<T>;
      if constexpr (!N.empty)
        s << " ";
    }

    // print name
    if constexpr (!N.empty)
      s << N;

    // print value
    if constexpr (!std::is_void_v<T>)
      s << " = " << bbm::toString(arg.value());

    // print default
    if constexpr (!std::is_void_v<D>)
      s << " [ = " << bbm::toString(D()()) << "]";

    return s;
  }

  /**********************************************************************/
  /*! \brief arg literal
   **********************************************************************/
  template<string_literal NAME>
    arg<void,NAME> operator""_arg(void)
  {
    return arg<void, NAME>{};
  }
  
} // end bbm namespace

/************************************************************************/
/*! \brief Helper Macro for creating the type of a lambda that returns a value
*************************************************************************/
#define ArgDef(value) decltype([](){return value; })

#endif /* _BBM_ARG_H_ */
