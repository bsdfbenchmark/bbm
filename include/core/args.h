#ifndef _BBM_ARGS_H_
#define _BBM_ARGS_H_

#include "core/arg.h"
#include "core/error.h"

#include "util/constfor.h"
#include "util/constforeach.h"
#include "util/to_string_literal.h"
#include "util/macro_util.h"
#include "util/make_from.h"

/************************************************************************/
/*! \file args.h
  \brief Provides a more flexible argument passing to function and methods.

  Example:
  
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  void foo( args<arg<float, "a">, arg<float, "b">> args)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
  defines a method foo that takes two arguments named "a" and "b" that each
  are a float. The basic way to call the method is:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo({1.0f, 2.0f});
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  note the additional enclosing curly brackets. See below for ways to remove
  the need for the brackets.  If there are multiple specializations of 'foo' then
  it might be needed to specify the type of the arguments:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo( args<arg<float,"a">, arg<float, "b">>{1.0f, 2.0f} );
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  However, other methods for passing the arguments are also supported:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo({"a"_arg = 1.0f, "b"_arg = 2.0f});
  foo({"b"_arg = 2.0f, "a"_arg = 1.0f});
  foo({1.0f, "b"_arg = 2.0f});
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  are also valid. Arguments can also be passed by their position in the args:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo({"0"_arg = 1.0f, "1"_arg = 2.0f});  // a = 1.0f, b = 2.0f;
  foo({"1"_arg = 2.0f, "0"_arg = 1.0f});  // a = 1.0f, b = 2.0f;
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
  More complex forms are possible depending on the argument
  defintions (see below).

  To acess the argument values in the method, one can use:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  args.template value<0>();   // zero-th argument.
  args.template value<"a">(); // argument named "a"
  args.value("a"_arg);        // argument named "a"
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Alternative, a helper macro is provided to create alias variables:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  BBM_IMPORT_ARGS(args, a, b);
  std::cerr << a << std::endl;
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  In the above case, an 'a' and 'b' alias is created to args.value("a"_arg)
  and agrs.value("b"_arg).

  Argument types can also be refernces (const and non-const):

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo(args<arg<float&, "a">, arg<const float&, "b">> args)
  {
    BBM_IMPORT_ARGS(args, a, b);
    std::cerr << b << std::endl;
    a = 10.0f;
  }
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  In this case, "a" is a reference to a float, and setting "a = 10.0f" will
  have the expected result that the value of the referenced variable is
  affected outside the function body.

  "b" is declared a const float& and supports the same functionality as in
  regular C++ calls. I.e., when an rvalue is passed, its scope is extended as
  long as 'args' scope is alive.

  Arguments can also be given a default value with 'ArgDef':

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo(args<arg<const float&, "a", ArgDef(1.0f)>, arg<const float&, "b", ArgDef(2.0f)>> args)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  In this case 'ArgDef' is macro that wraps the default value in a lambda
  function.  This means that the default value cannot depend on run-time
  information.

  Default values allow for other flexible method calling stratgies:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo({});                // a = 1.0f, b = 2.0f
  foo({10.0f});           // a = 10.0f, b = 2.0f
  foo({"b"_arg = 11.f});  // a = 1.0f, b = 11.0f
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  The algorithm for assigning arguments (implemented below in _find_index_arg)
  is that for each argument in args, the list of passed arguments is searched
  for a argument that matches one of the following rules (in order of
  precedence):
  
  + Use the argument if it has a matching name (and a compatible type); or
  + Use the argument if it has a numerical name that matches the position of the argument in args (and compatible type); or
  + Use the argument at the same position in the list of arguments if it has no name and a compatible type; or
  + The argument has a unique type and an argument with the same unique type is passed

  Note: a compile error will occur when:
  + Not all arguments passed can be assigned to an argument in args.
  + An argument with an invalid name is passed.
  

  An example of matching unique types is given below:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo(args<arg<uniqueA, "a">, arg<uniqueB, "b">> args)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  where 'uniqueA' and 'unqiueB' are unique types (i.e., uniqueA != uniqueB),
  such as differnt enum_types.  In this case one can call 'foo' as:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo({uniqueA(), uniqueB()});
  foo({uniqueB(), uniqueA()});
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Given an args definition, two helper static constexpr are defined to check
  whether a list of argument types is compatible:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  using args_type = args<arg<float, "a">, arg<float, "b", ArgDef(1.0f)>>;
  std::cerr << args_type::is_compatible<float, float> << std::endl;     // true
  std::cerr << args_type::is_compatible<float> << std::endl;            // true
  std::cerr << args_type::is_compatible<> << std::endl;                 // false
  std::cerr << args_type::is_compatible<arg<float, "a">> << std::endl;  // true
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  The helper 'is_cpp_compatible' returns true if the argument list is
  compatible according to classic c++ argument passing rules.

  
  To remove the curly brackets around the arguments, two helper macros are
  given: one for the case where the method is declared with an 'args'
  argument, and one where the method is declared in a classic C++ manner.

  Case 1: the method 'foo' has an 'args' argument.

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  BBM_FORWARD_ARGS(foo, arg<float, "a">, arg<float, "b">);
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
  forwards the call to 'foo' to the version with 'args':
  
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo("b"_arg = 2.0f, "0"_arg = 1.0f);
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  will forward to:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  foo(args<arg<float, "a">, arg<float, "b">{"b"_arg = 2.0f, "0"_arg = 1.0f});
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
  Note that the list of 'arg' needs to match the signature of the 'args' of
  'foo' to forward too.  A variant of this macro named
  'BBM_FORWARD_ARGS_CONST' operates exactly the same except that it declares
  the forward method as 'const'.

  Case 2: the method 'foo' has classic C++ style arguments:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  BBM_FORWARD_CPP_ARGS(foo, arg<float, "a">, arg<float, "b">)
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  thus exactly the same arguments as the previous case. Usage is exactly the
  same too.  The provided arg list must exactly match the types of the
  original 'foo' method signature, _including_ any default values!  If not,
  the forwarding method will not be able to detect when to use the original
  function (with C++ arguments) or when it needs to use an intermediate 'args'
  forwarding call (i.e., using the is_cpp_compatible static constexpr).
  Similar as with the previous case, 'BBM_FORWARD_CPP_ARGS_CONST' declares the
  forwarding method as 'const'.

  To forward constructor calls, two similar macros are defined:
  + BBM_CONSTRUCTOR_FORWARD_ARGS
  + BBM_CONSTRUCTOR_FORWARD_CPP_ARGS
  which use exactly the same set of arguments as for regular functions and
  operate in roughly the same way.
  
*************************************************************************/
namespace bbm {

  //! \brief Forward declaration
  template<typename... ARGS> requires (is_arg_v<ARGS> && ...) struct args;
  
  /**********************************************************************/
  /*! @{ \name Type Traits
   **********************************************************************/
  namespace detail {
    template<typename T> struct is_args_impl : std::false_type {};
    template<typename... Ts> struct is_args_impl<args<Ts...>> : std::true_type {};
  }

  template<typename T>
    using is_args = detail::is_args_impl<std::decay_t<T>>;

  template<typename T>
    inline constexpr bool is_args_v = is_args<T>::value;
  //! @}

  /**********************************************************************/
  /*! @{ \name Create a bbm::args type from either a list of bbm::arg or an bbm::args.
   **********************************************************************/
  namespace detail {
    template<typename... Ts> struct add_args_impl;
    template<typename... Ts> requires (is_arg_v<Ts> && ...) struct add_args_impl <Ts...> { using type = args<Ts...>; };
    template<typename T> requires is_args_v<T> struct add_args_impl<T> { using type = T; };
  }

  template<typename... Ts>
    using add_args = detail::add_args_impl<Ts...>;
  
  template<typename... Ts>
    using add_args_t = typename add_args<Ts...>::type; 
  //! @}
  
  /**********************************************************************/
  /*! \brief Main declaration of 'args'
   **********************************************************************/
  template<typename... ARGS> requires (is_arg_v<ARGS> && ...)
    struct args
  {
  private:
    //! @{ \name Helper forward functions
    template<typename... Ts> static constexpr bool _is_compatible_fwd(void) { return _is_compatible<std::tuple<Ts...>>(); }
    template<typename... Ts> static constexpr bool _is_cpp_compatible_fwd(void) { return _is_cpp_compatible<std::tuple<Ts...>>(); }
    //! @}
    
  public:
    /********************************************************************/
    /*! @{ \name compatibility checks of series of arguments with ARGS
     ********************************************************************/
    template<typename... Ts>
      static constexpr bool is_compatible = _is_compatible_fwd<Ts...>();
    
    template<typename... Ts>
      static constexpr bool is_cpp_compatible = _is_cpp_compatible_fwd<Ts...>();
    //! @}

    
    /********************************************************************/
    /*! @{ \name Constructors
    *********************************************************************/
    //! \brief Copy constructor for compatible arg_lists
    template<typename... Ts> requires is_compatible<Ts...>
      constexpr args(args<Ts...>&& arg) : _values( _retrieve_args(arg.values(), std::make_index_sequence<sizeof...(ARGS)>{}) ) {}
      
    //! \brief Constructor from a compatible list of arguments
    template<typename... Ts> requires is_compatible<Ts...>
      constexpr args(Ts&&... arg) : _values( _retrieve_args(std::forward_as_tuple(std::forward<Ts>(arg)...), std::make_index_sequence<sizeof...(ARGS)>{}) ) {}
    //! @}
    
    /********************************************************************/
    /*! @{ \name Inspectors
     ********************************************************************/
    static constexpr size_t size = sizeof...(ARGS);

    template<size_t IDX> using type = std::tuple_element_t<IDX, std::tuple<ARGS...>>;
    
    //! \brief Returns a tuple of all arguments' values
    inline constexpr auto values(void) const { return _value( std::make_index_sequence<sizeof...(ARGS)>{} ); }

    //! \brief Get the IDX-th argument (bbm::arg type)
    template<size_t IDX> inline constexpr decltype(auto) get(void) { return std::get<IDX>(_values); }
    template<size_t IDX> inline constexpr decltype(auto) get(void) const { return std::get<IDX>(_values); }
    
    //! \brief Get the value of the IDX-th argument
    template<size_t IDX> inline constexpr decltype(auto) value(void) { return get<IDX>().value(); }
    template<size_t IDX> inline constexpr decltype(auto) value(void) const { return get<IDX>().value(); }

    //! \brief Get the argument matching a given name (bbm::arg type)
    template<string_literal Name, typename T=void, typename D=void>
      inline constexpr decltype(auto) get(arg<T,Name,D> = arg<T,Name,D>{})
    {
      const size_t idx = _lookup<Name>();
      static_assert(idx < size, BBM_NO_MATCH);
      return std::get<idx>(_values);
    }

    template<string_literal Name, typename T=void, typename D=void>
      inline constexpr decltype(auto) get(arg<T,Name,D> = arg<T,Name,D>{}) const
    {
      const size_t idx = _lookup<Name>();
      static_assert(idx < size, BBM_NO_MATCH);
      return std::get<idx>(_values);
    }

    //! \brief Get the value of the argument with a given name
    template<string_literal Name, typename T=void, typename D=void> inline constexpr decltype(auto) value(arg<T,Name,D> = arg<T,Name,D>{}) { return get<Name>().value(); }
    template<string_literal Name, typename T=void, typename D=void> inline constexpr decltype(auto) value(arg<T,Name,D> = arg<T,Name,D>{}) const { return get<Name>().value(); }
    //! @}
    
    //  private:  DEBUG!!!
    /********************************************************************/
    /*! \brief Lookup a name.

      \param name = bbm::arg with name to look up
      \returns the index of the matching arg (or size if failed)
    *********************************************************************/
    template<string_literal Name, typename T=void, typename D=void>
      static inline constexpr size_t _lookup(arg<T, Name, D> = arg<T, Name, D>{})
    {
      size_t result = size;
      
      CONSTFOR(idx, sizeof...(ARGS),
      {
        // matching name?
        if constexpr (type<idx>::name == Name) result = idx;

        // name matches "idx"?
        if constexpr (to_string_literal<idx>() == Name) result = idx;
      });

      // Done (returns 'size' if fail)
      return result;
    }

    /********************************************************************/
    /*! \brief Populate the 'args' from a tuple of arguments

      \param src = tuple of argument values
      \param index_sequence [0, sizeof...(ARGS)]
      \returns tuple of values
     ********************************************************************/
    template<typename TUP, size_t... IDX>
      static inline constexpr decltype(auto) _retrieve_args(TUP&& src, std::index_sequence<IDX...>)
    {
      return std::make_tuple( _retrieve_arg<IDX>(std::forward<TUP>(src))... );
    }

    /********************************************************************/
    /*! \brief Find in a tuple of arguments the one that best matches the 'IDX'-th ARG in ARGS

      \tparam IDX = index in ARGS to find the best match
      \param src = tuple of argument values
      \return the matching ARG from TUP
    *********************************************************************/
    template<size_t IDX, typename TUP>
      static inline constexpr decltype(auto) _retrieve_arg(TUP&& src)
    {
      using arg_type = type<IDX>;

      // Find best matching element in TUP and returns its index
      constexpr size_t idx = _find_arg_index<IDX, TUP>();

      // if found, use value to create arg
      if constexpr (idx != std::tuple_size_v<TUP>)
      {
        decltype(auto) arg = std::get<idx>( std::forward<TUP>(src) );
        return arg_type(std::forward<decltype(arg)>(arg)); 
      }

      // else return empty (if available)
      else if constexpr (std::is_constructible_v<arg_type>) return arg_type{};

      // No match found and not default constructible => compile time error
      else static_assert(dependent_false_v<TUP>, BBM_NO_MATCH);
    }

    /********************************************************************/
    /*! \brief Checks if a tuple of arguments is compatible with ARGS

      \tparam TUP = tuple of argument types
      \returns true if the argument types in TUP form a valid initialization for ARGS
     ********************************************************************/
    template<typename TUP>
      static inline constexpr bool _is_compatible(void)
    {
      bool compatible = true;
      bool arg_used[ std::tuple_size_v<TUP> ];
      std::fill_n(arg_used, std::tuple_size_v<TUP>, false);

      // Check found argument in TUP for each ARGS
      CONSTFOR(idx, sizeof...(ARGS),
      {
        const size_t argIdx = _find_arg_index<idx, TUP>();

        // not found => check if default exists
        if constexpr (argIdx >= std::tuple_size_v<TUP>)
        {
          if constexpr (!std::is_constructible_v<type<idx>>) compatible = false;
        }

        // check if already assigned before
        else if(arg_used[ argIdx ]) compatible = false;
        
        // found => mark as used
        else arg_used[ argIdx ] = true;
      });

      // check if all arguments in TUP have been used
      return compatible && std::all_of(arg_used, arg_used + std::tuple_size_v<TUP>, [](bool b) { return b; });
    }


    /********************************************************************/
    /*! \brief Check if a tuple of arguments is compatible in a classic C++ sense with ARGS
    
      \tparam TUP = tuple of argument types
      \param index_sequence [0, sizeof...(ARGS)]
      \returns true if the argument types in TUP form a valid initialization for ARGS
     ********************************************************************/
    template<typename TUP>
      static inline constexpr bool _is_cpp_compatible(void)
    {
      bool compatible = true;
      bool defaulted = false;   // keeps track if a previous argument has been defaulted
      bool arg_used[ std::tuple_size_v<TUP> ];
      std::fill_n(arg_used, std::tuple_size_v<TUP>, false);
      
      CONSTFOR(idx, sizeof...(ARGS),
      {
        const size_t found_idx = _find_arg_index<idx, TUP>();

        // if found: no previous arg can be defaulted, and index must match, and arg element is !is_arg_v
        if constexpr (found_idx != std::tuple_size_v<TUP>)
        {
          arg_used[ found_idx ] = true;  // mark as used
          if(defaulted) compatible = false;
          if constexpr (found_idx != idx) compatible = false;
          if constexpr (is_arg_v< std::tuple_element_t<found_idx, TUP> >) compatible = false;
        }

        // if not found: type<idx> must be default constructible
        else
        {
          defaulted = true;
          if constexpr (!std::is_constructible_v<type<idx>>) compatible = false;
        }
      });

      // check if all arguments in TUP have been used
      return compatible && std::all_of(arg_used, arg_used + std::tuple_size_v<TUP>, [](bool b) { return b; });
    }
    
    /********************************************************************/
    /*! \brief Find the element in a tuple of argument values that best matches the IDX-th 'ARGS'

      \tparam IDX = index in 'ARGS' to match
      \tparam TUP = tuple of argument values
      \returns the index in TUP of the best matching element; returns sizeof(TUP) is no match is found.

      Matching procedure:
      + Check for matching name and compatible type
      + Check for matching "IDX" name in TUP with compatible type
      + If TUP[IDX] is type compatible
      + If ARGS[IDX] type is unique and there exists a unique matching type in TUP
      + Else return sizeof<TUP> (fail)
    *********************************************************************/
    template<size_t IDX, typename TUP>
      static inline constexpr size_t _find_arg_index(void)
    {
      size_t result = std::tuple_size_v<TUP>;
      using arg_type = type<IDX>;
        
      // if ARGS[IDX] has valid name => see if matching name exists in TUP (that is type compatible)
      if constexpr (!arg_type::name.empty)
      {
        CONSTFOR(tupIdx, std::tuple_size_v<TUP>,
        {
          using tup_type = std::tuple_element_t<tupIdx, TUP>;

          if constexpr (is_arg_v<tup_type>)
            if constexpr (std::decay_t<tup_type>::name == arg_type::name)
              if constexpr (std::convertible_to<typename std::decay_t<tup_type>::type, arg_type>) result = tupIdx;
        });

        // if found => terminate search
        if(result != std::tuple_size_v<TUP>) return result;
      }
      
      // ELSE: check if there exists an argument in TUP with name "IDX" (and is type compatible)
      CONSTFOR(tupIdx, std::tuple_size_v<TUP>,
      {
        using tup_type = std::tuple_element_t<tupIdx, TUP>;
        if constexpr (is_arg_v<tup_type>)
          if constexpr (std::decay_t<tup_type>::name == to_string_literal<IDX>())
            if constexpr (std::convertible_to<typename std::decay_t<tup_type>::type, arg_type>) result = tupIdx;
      });

      // if found => terminate search
      if(result != std::tuple_size_v<TUP>) return result;

      // ELSE: check if TUP[IDX] and ARGS[IDX] are type compatible
      if constexpr (IDX < std::tuple_size_v<TUP>)
      {
        using tup_type = std::tuple_element_t<IDX, TUP>;

        // handle non-bbm::arg arguments in TUP
        if constexpr (!is_arg_v<tup_type>)
        {
          if constexpr (std::convertible_to<tup_type, arg_type>) return IDX;
        }
        
        // argument in TUP is a bbm::arg (with an empty name!)
        else if constexpr (std::decay_t<tup_type>::name.empty)
          if constexpr (std::convertible_to<typename std::decay_t<tup_type>::type, arg_type>) return IDX;
      }

      // ELSE: if arg_type::type is unique, is there an exact matching (unique) type in TUP?
      size_t arg_count = 0;
      CONSTFOREACH(ARG, ARGS,
      {
        arg_count += std::is_same_v<std::decay_t<typename arg_type::type>, std::decay_t<typename ARG::type>>;
      });
      
      if(arg_count == 1)  // if unique arg_type
      {
        size_t tup_count = 0;
        size_t found_idx = std::tuple_size_v<TUP>;
        CONSTFOR(tupIdx, std::tuple_size_v<TUP>,
        {
          using tup_type = std::tuple_element_t<tupIdx, TUP>;
          if constexpr (std::is_same_v<std::decay_t<tup_type>, std::decay_t<typename arg_type::type>>)
          {
            tup_count++;
            found_idx = tupIdx;
          }
        });

        // if unique tup_type (that matches arg_type); return index
        if(tup_count == 1 && found_idx < std::tuple_size_v<TUP>) return found_idx; 
      }
      
      // ELSE: fail
      return result;
    }

    /********************************************************************/
    /*! \brief Return the value of all arguments

      \param index_sequence = [0, sizeof(ARGS)]
      \returns tuple of argument values 
     ********************************************************************/
    template<size_t... IDX>
      inline constexpr auto _value(std::index_sequence<IDX...>) const
    {
      return std::forward_as_tuple( value<IDX>()... );
    }
    
  private:
    ////////////////
    // Member Data
    ////////////////
    std::tuple<ARGS...> _values;
  };


  /**********************************************************************/
  /*! \brief Print bbm::args
   **********************************************************************/
  template<typename... ARGS> requires (bbm::is_arg_v<ARGS> && ...)
    std::ostream& operator<<(std::ostream& s, const bbm::args<ARGS...>& args)
  {
    CONSTFOR(idx, sizeof...(ARGS),
    {
      if constexpr (idx > 0) s << ", ";
      s << args.template get<idx>();
    });
    return s;
  }
} // end bbm namespace


/************************************************************************/
/*! \brief Macro for creating aliases of the arguments in args

  \param ARGS = name of args
  \param ... = comma separated list of argument names to import.
*************************************************************************/
#define BBM_IMPORT_ARGS(ARGS, ...)                                       \
  auto& _bbm_import_args = ARGS;                                         \
  BBM_FOREACH( _BBM_IMPORT_ARG_HELPER, __VA_ARGS__ )                     \
  
//! \brief Helper macro for BBM_IMPORT_ARGS
#define _BBM_IMPORT_ARG_HELPER(ARGNAME) BBM_IMPORT_ARG(_bbm_import_args, ARGNAME)

/************************************************************************/
/*! \brief Macro for creating an alias for a single arg in args

  \param ARGS = name of args
  \param ARGNAME = name of the arg to import (without "")
*************************************************************************/
#define BBM_IMPORT_ARG(ARGS, ARGNAME)                                    \
  decltype(auto) ARGNAME = ARGS.value(#ARGNAME ## _arg);                 \


/************************************************************************/
/*! \brief Macro for forwarding parameters (without {}) to a method with args

  \param NAME = method name
  \param CTE = 'const' or empty to declare the method const.
  \param ... = list of arg<> definitions (or a single args<> definition)

  Limitation: the class cannot have a template parameter named _ArgTs
*************************************************************************/
#define _BBM_FORWARD_ARGS(NAME, CTE, ...)                                \
  template<typename... _ArgTs> requires                                  \
    (bbm::add_args_t<__VA_ARGS__>::template is_compatible<_ArgTs...> &&  \
     !(bbm::is_args_v<_ArgTs> && ...))                                   \
    inline constexpr auto NAME(_ArgTs&&... t) CTE                        \
  {                                                                      \
    return NAME(bbm::add_args_t<__VA_ARGS__>{std::forward<_ArgTs>(t)...}); \
  }                                                                      \

//! \brief Forward arguments (without {}) to a method with args
#define BBM_FORWARD_ARGS(NAME, ...) BBM_CALL(_BBM_FORWARD_ARGS, NAME, , __VA_ARGS__)

//! \brief Forward arguments (without {}) to a _const_ method with args
#define BBM_FORWARD_ARGS_CONST(NAME, ...) BBM_CALL(_BBM_FORWARD_ARGS, NAME, const, __VA_ARGS__)


/************************************************************************/
/*! \brief Macro for forwarding parameters via args to a regular CPP method.

  \param NAME = method name
  \param CTE = 'const' or empty to declare the method const.
  \param ... = list of arg<> definitions (or a single args<> definition)

  Limitation: the class cannot have a template parameter named _ArgTs
*************************************************************************/
#define _BBM_FORWARD_CPP_ARGS(NAME, CTE, ...)                            \
  template<typename... _ArgTs> requires                                  \
     (!bbm::add_args_t<__VA_ARGS__>::template is_cpp_compatible<_ArgTs...> && \
      bbm::add_args_t<__VA_ARGS__>::template is_compatible<_ArgTs...> && \
      !(bbm::is_args_v<_ArgTs> && ...))                                  \
    inline constexpr auto NAME(_ArgTs&&... t) CTE                        \
  {                                                                      \
    auto launcher = [&]<typename ARGS, size_t... IDX>(ARGS&& args, std::index_sequence<IDX...>) { return NAME(args.template value<IDX>()...); }; \
    return launcher(bbm::add_args_t<__VA_ARGS__>{std::forward<_ArgTs>(t)...}, std::make_index_sequence<bbm::add_args_t<__VA_ARGS__>::size>{}); \
  }                                                                      \

//! \brief Forward arguments via args to a method with CPP args.
#define BBM_FORWARD_CPP_ARGS(NAME, ...) BBM_CALL(_BBM_FORWARD_CPP_ARGS, NAME, , __VA_ARGS__)

//! \brief Forward arguments via args to a _const_ method with CPP args.
#define BBM_FORWARD_CPP_ARGS_CONST(NAME, ...) BBM_CALL(_BBM_FORWARD_CPP_ARGS, NAME, const, __VA_ARGS__)


/************************************************************************/
/*! \brief Macro for forwarding parameters (without {}) to a constructor with args

  \param NAME = class name
  \param ... = list of arg<> definitions (or a single args<> definition)

  Remarks:
  + Avoid hijacking the copy constructor by ignoring single arguments of the same type as the class
  + A default (empty) constructor is created if all arg have an associated default value.

  Limitation: the class cannot have a template parameter named _ArgTs
*************************************************************************/
#define _BBM_CONSTRUCTOR_FORWARD_ARGS(NAME, ...)                         \
  template<typename... _ArgTs> requires                                  \
    (bbm::add_args_t<__VA_ARGS__>::template is_compatible<_ArgTs...> &&  \
     !(bbm::is_args_v<_ArgTs> && ...) &&                                 \
     !(sizeof...(_ArgTs) == 1 && (std::is_same_v<NAME, std::decay_t<_ArgTs>> && ...))) \
    inline constexpr NAME(_ArgTs&&... t)                                 \
      : NAME(bbm::add_args_t<__VA_ARGS__>{std::forward<_ArgTs>(t)...}) {}\
                                                                         \
  template<typename=void> requires bbm::add_args_t<__VA_ARGS__>::template is_compatible<> \
    inline constexpr NAME(void) : NAME( bbm::make_from<bbm::add_args_t<__VA_ARGS__>>()) {} \

//! \brief Forward arguments (without {}) to a constructor with args
#define BBM_CONSTRUCTOR_FORWARD_ARGS(...) BBM_CALL(_BBM_CONSTRUCTOR_FORWARD_ARGS, __VA_ARGS__)


/************************************************************************/
/*! \brief Macro for forwarding parameters (without {}) to a constructor with regular C++ args

  \param NAME = class name
  \param ... = list of arg<> definitions (or a single args<> definition)

  Remarks:
  + Avoid hijacking the copy constructor by ignoring single arguments of the same type as the class
  + A helper constructor that takes args and an index_sequence is created.
  + There is no need for a default constructor as this will be handled (if necessary) by the original constructor.

  Limitation: the class cannot have a template parameter named _ArgTs or _ArgIdx
*************************************************************************/
#define _BBM_CONSTRUCTOR_FORWARD_CPP_ARGS(NAME, ...)                     \
  template<size_t... _ArgIdx> requires (sizeof...(_ArgIdx) == bbm::add_args_t<__VA_ARGS__>::size) \
    inline constexpr NAME(bbm::add_args_t<__VA_ARGS__>&& args, std::index_sequence<_ArgIdx...>) \
      : NAME(args.template value<_ArgIdx>()...) {}                       \
                                                                         \
  template<typename... _ArgTs> requires                                  \
     (!bbm::add_args_t<__VA_ARGS__>::template is_cpp_compatible<_ArgTs...> && \
      bbm::add_args_t<__VA_ARGS__>::template is_compatible<_ArgTs...> && \
      !(bbm::is_args_v<_ArgTs> && ...) &&                                \
      !(sizeof...(_ArgTs) == 1 && (std::is_same_v<NAME, std::decay_t<_ArgTs>> && ...))) \
    inline constexpr NAME(_ArgTs&&... t)                                     \
      : NAME(bbm::add_args_t<__VA_ARGS__>{std::forward<_ArgTs>(t)...}, std::make_index_sequence<bbm::args<__VA_ARGS__>::size>{}) {} \
                                                                         \
//! \brief Forward arguments (without {}) to a constructor with C++ args.
#define BBM_CONSTRUCTOR_FORWARD_CPP_ARGS(...) BBM_CALL(_BBM_CONSTRUCTOR_FORWARD_CPP_ARGS, __VA_ARGS__)


#endif /* _BBM_ARGS_H_ */
