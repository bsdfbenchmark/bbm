#ifndef _BBM_REFLECTION_H_
#define _BBM_REFLECTION_H_

#include "util/tuple.h"
#include "util/named.h"
#include "util/macro_util.h"
#include "util/string_literal.h"

#include "concepts/reflection.h"

/************************************************************************/
/*! \file reflection.h
  \brief Compile-time reflection of:

  + class attributes (types and run-time values)
  + base classes

  To do:
  + member functions

  Usage:

  To add attribute reflection, add 'BBM_ATTRIBUTES(<attribute names...>)' after
  the declaration of the class attributes. For example:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  struct foo
  {
    floar bar;
    char var;

    BBM_ATTRIBUTES(bar, var);
  };
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  We can then query the attributes, types, and number of attributes:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo f{1,'a'};
  std::cout << attributes(f) << std::endl;                                           // (bar = 1, var = 'a')
  std::cout << typestring< attributes_t<foo> > << std::endl;                         // named< std::tuple<float&, char&>, "bar", "var" >
  std::cout << attributes_size<foo> << std::endl;                                    // 2
  std::cout << typestring< std::tuple_element_t<0, attributes_t<foo>>> << std::endl; // float
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Note: we can also pass the attributes of an attribute; the name will be
  the names of the attributes of of the attribue:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  struct foo2
  {
     float a;
     foo b;
     BBM_ATTRIBUTES(a, attributes(b));
  };
  foo2 f2;
  std::cout << attributes(f2) << std::endl;    // (a=0, bar=0, var=0)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  To add public base class reflection, add 'BBM_BASETYPES(<base class names...>)'
  at the start of the class. For example:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  struct foo2 : public foo
  {
    BBM_BASETYPES(foo);
    foo2(float a, float b, char c) : bar2(a), bar(b), var(c) {}
    float bar2;
    BBM_ATTRIBUTES(bar2);
  }
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  To querry the type of the return type of attributes(...):

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  struct foo3
  {
     float a, b;
     BBM_ATTRIBUTES(a,b);

     using A = BBM_ATTRIBUTES_T;     // bbm::named<std::tuple<float,float>, "a", "b">
  };

  using B = attributes_t<foo3>;      // bbm::named<std::tuple<float,float>, "a", "b">
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  The difference between both (A and B) is that B requires an evaluated
  context (i.e., the type of the class must be known), whereas A is robust in
  unevaluated contexts (i.e., from inside the class before the class is
  complete).
  
  We can query the base classes:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  std::cout << typestring< std::tuple_element_t< 0, basetypes_t<foo2> > > << std::endl; // foo
  std::cout << basetypes_size<foo2> << std::endl;                                       // 1
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Querying the attributes will include the attributes of the base class too:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  std::cout << attributes(f2) << std::endl;  // (bar2 = 3, bar = 1, var = 'a')
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  In some cases it might be desirable to prefix the attribute names of a base class:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  struct foo3 : public foo, foo2
  {
     BBM_BASETYPES(prefix<"foo::", foo>, foo2);
     foo3(float a, float b, char c) : foo(13,'b'), foo2(a,b,c) {}
  };

  foo3 f3(3,1,'a');
  std::cout << attributes(f3) << std::endl;   // (foo::bar = 13, foo::var = 'b', bar2 = 3, bar = 1, var = 'a')
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
*************************************************************************/

namespace bbm {
  namespace reflection {

    /********************************************************************/
    /*! \brief BBM_ATTRIBUTES macro for declaring the attriute of a class
     ********************************************************************/
#define BBM_ATTRIBUTES(...)                                             \
    using attribute_tuple_t = bbm::named_cat_t< decltype(bbm::named_flatten(bbm::make_named<BBM_STRINGIFY_EACH(__VA_ARGS__)>(bbm::make_ref_tuple(__VA_ARGS__)))), typename reflection_base_t::attribute_tuple_t>; \
                                                                        \
    inline constexpr auto attribute_tuple(void)                         \
    {                                                                   \
      auto attr = bbm::named_flatten(bbm::make_named<BBM_STRINGIFY_EACH(__VA_ARGS__)>(bbm::make_ref_tuple(__VA_ARGS__))); \
      return bbm::named_cat(attr, reflection_base_t::attribute_tuple(*this)); \
    }                                                                   \
                                                                        \
    inline constexpr auto attribute_tuple(void) const                   \
    {                                                                   \
      auto attr = bbm::named_flatten(bbm::make_named<BBM_STRINGIFY_EACH(__VA_ARGS__)>(bbm::make_ref_tuple(__VA_ARGS__))); \
      return bbm::named_cat(attr, reflection_base_t::attribute_tuple(*this)); \
    }                                                                   \
    

    /*******************************************************************/
    /*! \brief BBM_ATTRIBUTES_T macro for robustly obtaining the type of the
        attributes tuple in _unevaluated_ context. 
     *******************************************************************/
#define BBM_ATTRIBUTES_T std::conditional_t<attribute_tuple_t::size == 0, typename reflection_base_t::attribute_tuple_t, attribute_tuple_t>

    
    /*** Implementationd details ***/
    namespace detail {
      template<typename T> struct attributes_impl
      {
        using type = bbm::named<std::tuple<>>;
        static inline constexpr auto value(T&&) { return type{}; }
      };

      //! \brief void specialization
      template<> struct attributes_impl<void>
      {
        using type = bbm::named<std::tuple<>>;
      };
    
      //! \brief Specialization: has attributes => attribute_tuple
      template<typename T> requires concepts::reflection::attributes<T>
        struct attributes_impl<T>
      {
        using type = typename std::decay_t<T>::attribute_tuple_t;
        static inline constexpr auto value(T&& t) { return t.attribute_tuple(); }        
      };

      //! \brief Specialization: has basetype, but no attributes => reflection_base::attribute_tuple
      template<typename T> requires (concepts::reflection::basetypes<T> && !concepts::reflection::attributes<T>)
        struct attributes_impl<T>
      {
        using type = typename std::decay_t<T>::reflection_base_t::attribute_tuple_t;
        static inline constexpr auto value(T&& t) { return std::decay_t<T>::reflection_base_t::attribute_tuple(std::forward<T>(t)); }
      };
    } // end detail namespace
 
    
    /********************************************************************/
    /* @{ \name Attribute relfection methods
    *********************************************************************/
    template<typename T>
      inline constexpr decltype(auto) attributes(T&& t)
    {
      return bbm::reflection::detail::attributes_impl<T>::value(std::forward<T>(t));
    }
    
    template<typename T>
      using attributes_t = typename bbm::reflection::detail::attributes_impl<T>::type;
    
    template<typename T>
      static constexpr size_t attributes_size = std::tuple_size< attributes_t<T> >::value;
    //! @}

    /********************************************************************/
    /*! \brief BaseTypes macro for declaring the base types/classes of a class
     ********************************************************************/
#define BBM_BASETYPES(...) using reflection_base_t = bbm::reflection::detail::base_types<__VA_ARGS__>;

    
    /********************************************************************/
    /*! \brief Wrapper for adding a prefix to the attribute names of a base type.
     ********************************************************************/
    template<string_literal N, typename C> struct prefix {}; 


    /*** Implementation detail for reflection_base_t ***/
    namespace detail {

      //! \brief base_type wrapper; default case base type/class has no attribute reflection
      template<typename Cs>
        struct base_type
      {
        using type = std::tuple<Cs>;
        using attribute_tuple_t = bbm::named<std::tuple<>>;

        template<typename C> static inline constexpr auto attribute_tuple(C&) { return attribute_tuple_t{}; }
      };

      //! \brief base_type specialization when a base class does not have attribute reflection, but its parent might have
      template<typename Cs> requires (!concepts::reflection::attributes<Cs> && concepts::reflection::basetypes<Cs>)
        struct base_type<Cs>
      {
        using type = std::tuple<Cs>;
        using attribute_tuple_t = typename std::decay_t<Cs>::reflection_base_t::attribute_tuple_t;

        template<typename C>
          static inline constexpr auto attribute_tuple(C& This)
        {
          return std::decay_t<Cs>::reflection_base::attribute_tuple(This);
        }
      };

      //! \brief base_type specialization for when the base class also has attribute reflection
      template<typename Cs> requires concepts::reflection::attributes<Cs>
        struct base_type<Cs>
      {
        using type = std::tuple<Cs>;
        using attribute_tuple_t = typename std::decay_t<Cs>::attribute_tuple_t;

        template<typename C>
          static inline constexpr auto attribute_tuple(C& This)
        {
          if constexpr (is_const_v<C>) return bbm::reflection::attributes( static_cast<const Cs&>(This) );
          else return bbm::reflection::attributes( static_cast<Cs&>(This) );
        }
      };

      //! \brief base_type specialization for when a prefix is added to the base type's attributes
      template<string_literal N, typename Cs>
        struct base_type<prefix<N, Cs>>
      {
        using type = std::tuple<typename base_type<Cs>::type>;
        using attribute_tuple_t = bbm::prefix_names_t<N, typename base_type<Cs>::attribute_tuple_t >;
        
        template<typename C>
          static inline constexpr auto attribute_tuple(C& This)
        {
          return bbm::prefix_names<N>( base_type<Cs>::attribute_tuple(This) );
        }
      };

      //! \brief Container class for multiple base_type
      template<typename... Cs>
        struct base_types
      {
        using type = bbm::tuple_cat_t<typename base_type<Cs>::type... >;
        using attribute_tuple_t = bbm::named_cat_t< typename base_type<Cs>::attribute_tuple_t... >;

        template<typename C>
          static inline constexpr auto attribute_tuple(C& This)
        {
          return bbm::named_cat( base_type<Cs>::attribute_tuple(This)... );
        }
      };

      //! \brief Container specialization for classes without base types.
      template<>
        struct base_types<>
      {
        using type = std::tuple<>;
        using attribute_tuple_t = bbm::named<std::tuple<>>;
        template<typename C>  static inline constexpr auto attribute_tuple(C&) { return attribute_tuple_t{}; }
      };
    } // end detail


    /*** Implementation Details ***/
    namespace detail {

      template<typename C> struct basetypes_impl { using type = std::tuple<>; };

      template<typename C> requires bbm::concepts::reflection::basetypes<C>
        struct basetypes_impl<C>
      {
        using type = typename std::decay_t<C>::reflection_base_t::type;
      };

    } // end detail namespace
    
    /********************************************************************/
    /*! @{ \name BaseTypes reflection methods
     ********************************************************************/
    template<typename C>
      using basetypes_t = typename bbm::reflection::detail::basetypes_impl<C>::type;

    template<typename C>
      static constexpr size_t basetypes_size = std::tuple_size< basetypes_t<C> >::value;
    //! @}
    
  } // end reflection namespace
} // end bbm namespace

//! @{ \name Global aliases.
//!    Fallback for classes when no BaseTypes or Attributes are defined.  The
//!    reason for a global alias is because decltype(*this) cannot be used in
//!    an unevaluated context (i.e., attribute_tuple_t in Attributes(...).
using reflection_base_t = bbm::reflection::detail::base_types<>;
using attribute_tuple_t = reflection_base_t::attribute_tuple_t;
//! @}

#endif /* _BBM_REFLECTION_H_ */
