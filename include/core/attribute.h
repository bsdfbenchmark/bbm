#ifndef _BBM_ATTRIBUTE_H_
#define _BBM_ATTRIBUTE_H_

#include <type_traits>

#include "concepts/attribute.h"
#include "concepts/convertible.h"

/***********************************************************************/
/*! \file attribute.h

    \brief A warpper class to attach a properties to a class via a
    property struct.  The propert expects by default at least
    a type.
    
************************************************************************/

namespace bbm {

  //! \brief Base declaration of attribute; further specialized below.
  template<typename CONF> struct attribute;

  /**********************************************************************/
  /*! \brief Attributes for non-scalars (leverage inheritance)

    \tparam PROP = attribute property
    
    Implements: concepts::attribute

  ***********************************************************************/
  template<typename PROP> requires (concepts::attribute_property<PROP> && !std::is_scalar_v<typename PROP::type>)
    struct attribute<PROP> : public PROP::type
  {
    using type = typename PROP::type;
    using prop = PROP;
    
    //! \brief Default construstor
    inline attribute(void) = default;
    
    //! \brief Copy Constructor
    template<typename T> requires concepts::attribute<T>
      inline attribute(const T& t)
    {
      if constexpr (concepts::convertible<attribute, T>) convert(*this, t);  // call user-defined conversion
      else value() = t;
    }

    //! \brief Forward constructor
    template<typename... U> requires ((sizeof...(U) != 1) || (!concepts::attribute<U> && ...)) && requires(U&&... u) {{ type(std::forward<U>(u)...)};}
      inline attribute(U&&... u) : type(std::forward<U>(u)...) {}

    //! @{ \name Cast operators
    template<typename TYPE> requires std::constructible_from<TYPE, type>
      explicit inline operator TYPE(void) const { return TYPE(type(*this)); }

    inline operator type&(void) { return *this; }
    inline operator const type&(void) const { return *this; }
    
    inline type& value(void) { return *this; }
    inline const type& value(void) const { return *this; }
    //! @}
    
    //! \brief Assignment operator
    template<typename T> attribute& operator=(const T& t) { value() = attribute(t).value(); return *this; }
    
    //! @{ \name Operator forwarding
    inline decltype(auto) operator->(void) { return type::operator->(); }
    inline decltype(auto) operator->(void) const { return type::operator->();  }
    
    template<typename S> inline decltype(auto) operator->*(S s) { return type::operator->*(s); }
    template<typename S> inline decltype(auto) operator->*(S s) const { return type::operator->*(s); }
    //! @}

    //! \brief Custom toString
    inline std::string toString(void) const { return bbm::toString(value()); }
  };


  /**********************************************************************/
  /*! \brief Attribute specialization for scalar types for which we
    cannot use inheritance.
    
    \tparam PROP = attribute property
    
    Implements: concepts::attribute
    
  ***********************************************************************/
  template<typename PROP> requires (concepts::attribute_property<PROP> && std::is_scalar_v<typename PROP::type>)
    struct attribute<PROP>
  {
    using type = typename PROP::type;
    using prop = PROP;
    
    //! \brief Default construstor
    inline attribute(void) = default;

    //! \brief Value constructor
    inline attribute(const type& t) { _value = t; }
    
    //! \brief Copy Constructor
    template<typename T>  requires concepts::attribute<T>
      inline attribute(const T& t)
    {
      if constexpr (concepts::convertible<attribute, T>) convert(*this, t);
      else value() = t;
    }

    //! @{ \name Cast operators
    template<typename TYPE> requires std::constructible_from<TYPE,type>
      explicit inline operator TYPE(void) const { return TYPE(_value); }
    
    inline operator type&(void) { return _value; }
    inline operator const type&(void) const { return _value; }
    
    inline type& value(void) { return _value; }
    inline const type& value(void) const { return _value; }
    //! @}
    
    //! \brief Assignment operator
    template<typename T> attribute& operator=(const T& t) { value() = attribute(t).value(); return *this; }
    
    //! @{ \name Forward pointer operators
    inline decltype(auto) operator->(void) { return _value; }
    inline decltype(auto) operator->(void) const { return _value; }
    
    template<typename S> inline decltype(auto) operator->*(S s) { return _value->*(s); }
    template<typename S> inline decltype(auto) operator->*(S s) const { return _value->*(s); }
    //! @}

    //! \brief Custom toString
    inline std::string toString(void) const { return bbm::toString(value()); }
    
  private:
    type _value;
  };

  /**********************************************************************/
  /*! \brief return the value of an attribute, or if not an attribute the object
    *********************************************************************/
  template<typename T>
    decltype(auto) value(T&& t)
  {
    if constexpr (concepts::attribute<std::decay_t<T>>) return t.value();
    else return t;
  }

  
  /*** Implementation detail ***/
  namespace detail {
    template<typename T> struct attribute_value { using type = T; };
    template<typename T> requires bbm::concepts::attribute<T>
      struct attribute_value<T>
    {
      using type = typename std::decay_t<T>::type;
    };
  } // end detail namespace

  /**********************************************************************/
  /*! \brief return the type of value(t)
   **********************************************************************/
  template<typename T>
    using attribute_value_t = bbm::detail::attribute_value<std::decay_t<T>>::type;

  /**********************************************************************/
  /*! @{ \name Friend operators with attributes

    \brief Handle the case of 'scalar OP attribute' by explicitely casting
    attribute to its value.
    
   **********************************************************************/
#define BBM_ATTR_POST(OP) template<typename U, typename P> requires std::is_scalar_v<U> inline constexpr auto operator OP (U&& u, const attribute<P>& a) { return (std::forward<U>(u) OP a.value()); }

  BBM_ATTR_POST(+)
  BBM_ATTR_POST(-)
  BBM_ATTR_POST(*)
  BBM_ATTR_POST(/)
  BBM_ATTR_POST(%)

  BBM_ATTR_POST(&)
  BBM_ATTR_POST(|)
  BBM_ATTR_POST(^)
  BBM_ATTR_POST(&&)
  BBM_ATTR_POST(||)
  
  BBM_ATTR_POST(<)
  BBM_ATTR_POST(<=)
  BBM_ATTR_POST(>)
  BBM_ATTR_POST(>=)
  BBM_ATTR_POST(==)
  BBM_ATTR_POST(!=)
  
  #undef BBM_ATTR_POST
  //! @}
  
} // end bbm namespace



#endif /* _BBM_ATTRIBUTE_H_ */
