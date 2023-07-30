#ifndef _BBM_CONSTRUCTOR_H_
#define _BBM_CONSTRUCTOR_H_

#include "concepts/bsdf_attribute.h"

#include "util/type_traits.h"
#include "util/reflection.h"

#include "core/args.h"


/************************************************************************/
/*! \file constructor.h
  \brief Tools for easy creation of a constructors

  Case 1: a constructor that initializes all class attributes (requires bbm::refletion::attribute<T>)

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  BBM_DEFAULT_CONSTRUCTOR(foo)
  {
    // ...body...
  }
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  where 'foo' is the class name, and body is a custom constructor body
  executed after Attributes have been declared.  The
  constructor uses bbm::args to pass the arguments.

  Case 2: declare a constructor that takes a list of bbm::arg

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  BBM_CONSTRUCTOR(foo, args, ...)
  {
    // ...body...
  }
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  where 'foo' is again the class name, 'args' is the name given to the
  bbm::args passed as constructor arguments, followed by a list of bbm::arg
  (or a single bbm::args).
  
  
*************************************************************************/
namespace bbm {

  /*** Implementation detals of attribute_tuple_to_args_t ***/
  namespace detail {

    /*** Implementation details of converting a type and name into a bbm::arg ***/
    template<typename T, string_literal N> 
      struct attribute_to_arg_impl             // Default case
    {
      using type = bbm::arg<const std::decay_t<T>&, N>;
    };

    template<typename T, string_literal N> requires concepts::bsdf_attribute<T>
      struct attribute_to_arg_impl<T,N>       // T = bsdf_attribute
    {
      using type = bbm::arg<const typename std::decay_t<T>::type&, N, ArgDef(std::decay_t<T>::prop::default_value())>;
    };

    //! \brief Convert a type,name combination into a bbm::arg
    template<typename T, string_literal N>
      using attribute_to_arg = typename attribute_to_arg_impl<T,N>::type;
    
    template<typename T> struct attribute_tuple_to_args_t;

    /*** Implementation detail for converting a named tuple into a bbm::args ***/
    template<typename... Ts, string_literal... Ns>
    struct attribute_tuple_to_args_t< named<std::tuple<Ts...>, Ns...> >
    {
      using type = bbm::args<attribute_to_arg<Ts,Ns>...>;
    };
    
  } // end detail namespace
  
  /**********************************************************************/
  /*! \brief Convert an attribute_tuple_t to a bbm::args

    \tparam TUP = attribute_tuple_t (named tuple)

    Each attribute type and name in the named tuple is converted to an
    bbm::arg and wrapped in a bbm::args.  Each type,name combination is
    converted according to the following policies:
    + default: bbm::arg<const type&, name>
    + bsdf_attribute:  bbm::arg<const type&, name, bsdf_attribute::default_value>
    Other policies can be added to 'attribute_to_arg_impl.
  ***********************************************************************/
  template<typename TUP> requires is_named_v<TUP>
    using attribute_tuple_to_args_t = typename bbm::detail::attribute_tuple_to_args_t<std::decay_t<TUP>>::type;
  
} // end bbm namespace

/************************************************************************/
/*! \brief Helper macro to define a default constructor based on Attribute reflection

  \param ClassName = name of the constructor/class
  
  Creates a constructor that initializes the reflected attributes. The
  constructor uses bbm::args to pass the constructor arguments. The bbm::args
  signature is stored in an accessible constructor_args_t alias.

  The constructor will copy the corresponding bbm::args to the reflected
  attributes in the constructor body.

  Limitations:
  + The constructor can only be defined __after__ the Attributes (and reflection)
    have been declared.
  + The declaration must be followed by a function body (constructor_post_init)
    that is called after all Attributes have been set.
  + All other attributes/parent classes are initialized with an trivial
    constructor.

  Example:
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  BBM_DEFAULT_CONSTRUCTOR(foo)
  {
    // ...body...
  }
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**************************************************************************/
#define BBM_DEFAULT_CONSTRUCTOR(ClassName)                                \
  using constructor_args_t = bbm::attribute_tuple_to_args_t< BBM_ATTRIBUTES_T >; \
                                                                          \
  inline ClassName(const constructor_args_t& args)                        \
  {                                                                       \
    auto const_cast_tuple = []<typename TUP, size_t... IDX>(TUP&& tup, std::index_sequence<IDX...>) \
    {                                                                     \
      return std::forward_as_tuple( const_cast<std::decay_t<decltype(std::get<IDX>(tup))>&>( std::get<IDX>(tup) )... ); \
    };                                                                    \
                                                                          \
    const_cast_tuple(bbm::reflection::attributes(*this), std::make_index_sequence<bbm::reflection::attributes_size<decltype(*this)>>{}) = args.values(); \
    constructor_post_init();                                              \
  }                                                                       \
                                                                          \
  BBM_CONSTRUCTOR_FORWARD_ARGS(ClassName, constructor_args_t);            \
                                                                          \
  inline void constructor_post_init(void)                                 \



/*************************************************************************/
/*! \brief Helper macro for declaring a constructor based on a list of bbm::arg

  \param ClassName = name of constructor/class
  \param Args = name to give to bbm::args to access the arguments.
  \param ... = lost of bbm::arg (or a defintion of a single bbm::args)
               used to define the arguments to the constructor

  Creates a constructor (with forwarding) that takes a single bbm::args as
  input.  Use BBM_IMPORT_ARGS to create aliases in the body of the constructor
  of the passed arguments.  A constructor_args_t is also created with the signature
  of the bbm::args.

  Example:
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  BBM_CONSTRUCTOR(foo, args, bbm::arg<float, "a">, bbm::arg<float, "b">)
  {
    BBM_IMPORT_ARGS(args, a, b);
    // ...body... 
  }
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**************************************************************************/
#define BBM_CONSTRUCTOR(ClassName, Args, ...)                             \
  using constructor_args_t = bbm::add_args_t<__VA_ARGS__>;                \
                                                                          \
  BBM_CONSTRUCTOR_FORWARD_ARGS(ClassName, constructor_args_t);            \
                                                                          \
  inline ClassName(const constructor_args_t& Args)                        \


#endif /* _BBM_CONSTRUCTOR_H_ */


