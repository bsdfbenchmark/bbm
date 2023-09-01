#ifndef _BBM_NAMED_H_
#define _BBM_NAMED_H_

#include <tuple>

#include "concepts/util.h"

#include "util/constfor.h"
#include "util/string_literal.h"

/************************************************************************/
/*! \file named.h

  \brief A wrapper for STL containers such as tuple, pair, and array.  These
  containers force the programmer to remember the function/meaning of each
  element based on its position.  This is error-prone.  To mitigate this, the
  named wrapper allows to assign a name, via a template parameters, to each
  element so that subsequently it can be used to query elements by name, and
  to ensure that the assignment takes in account the order of the elements.

  **named container elements vs structs**  A struct cannot be defined in a
  function signature, and thus requires an external struct definition.  This
  is cumbersome for single use structures. Structured bindings introduced in
  C++17 do not completely solve this problem, since the definition is still
  annonymous. Example:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  struct foo { float a; float b; char c; };

  foo bar1(void);
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Alternatively, a single use tuple can be used:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  std::tuple<float, float, char> bar2(void);
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  but now the meaning of each element is implicit (is 'a' the first or second
  float?).  Named containers provide an alternative:
  
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  named<std::tuple<float,float,char>, "a", "b", "c"> bar3(void);
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Equivallent correspondling look ups for the above 3 stratgies are:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  auto b1 = bar1();
  auto b2 = bar2();
  auto b3 = bar3();

  std::cout << b1.a << std::endl;
  std::cout << std::get<0>(b2) << std::endl;
  std::cout << get<"a">(b3) << std::endl;
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  While a bit more verbose, named containers are more closely related to
  structs, and thus less error prone.
  
  Addtionally, named containers also allow for 'tie' with names:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  float a, b; char c;
  tie<"a", "b", "c">{a,b,c} = bar3();
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  The following will also take the order of names in account:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  named<std::tuple<float, char, float>, "b", "c", "a"> b4 = bar3();

  std::cout << get<"a">(b4) << " == " << get<"a">(b3) << std::endl;
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Similarly there are 2 helper methods for making a named structure:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  auto c1 = make_named<"a", "b", "c">( std::tuple<float,float,char>(1,2,'a') );
  auto c2 = make_named<"a", "b", "c">(1, 2, 'a');  // produces a named tuple
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  To pick a subset of named elements use:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  auto c3 = pick<"a", "c">(c2);
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
  All of this can be combined with structured bindings:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  auto [a, c] = pick<"a", "c">( foo3() );
  auto [e,f,g] = foo3();
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  The former has the preference, since it allows to:
  + pick a subset (i.e., skip elements) as demonstrated
  + explicitely name the elements you pick.

  Named structures can also be recursive; a shortcut for get allows to
  directly query the recursive values:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  auto c4 = make_named<"a">(make_named<"b", "c">('b', 'c'));
  std::cout << get<"a">(c4) << std::endl;        //   regular get: (b = b, c = c)
  std::cout << get<"a", "b">(c4) << std::endl;   // recursive get:  b
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Note: a recusive named type wraps the inner named type in a tuple.  Without
  wrapping, the names of the inner type are overwritten:

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
  named< named<std::tuple<int,int>, "a", "b">, "c", "d" > t0{1,2};       // (c = 1, d = 2)
  named< named<std::tuple<int,int>, "a", "b">, "c" >  t1;                // ERROR
  named< std::tuple<named<std::tuple<int,int>, "c", "d">>, "e"> t2(t0);  // (e = (c = 1, d = 2))
  named< std::tuple<named<std::tuple<int,int>, "c", "d">>, "e", "f"> t3; // ERROR
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
*************************************************************************/
  
namespace bbm {

  /**********************************************************************/
  /*! \brief named container

    \tparam T = underlying container type; must support std::get and std::tuple_size_v
    \tparam NAMES = names of each element
  ***********************************************************************/
  template<typename T, string_literal... NAMES> requires (sizeof...(NAMES) == 0) || (concepts::gettable<T> && (sizeof...(NAMES) == std::tuple_size_v<std::decay_t<T>>))
    struct named : public T
  {
  public:
    using value_type = T;

    //! \brief query name by index
    template<size_t IDX> requires (IDX < sizeof...(NAMES))
      static constexpr auto name = std::get<IDX>(std::make_tuple(NAMES...));

    //! \brief tuple of all names
    static constexpr auto names = std::make_tuple(NAMES...);

    //! \brief check if has_name 
    template<string_literal NAME>
      static constexpr bool has_name = (named<T,NAMES...>::template _find_name<0,NAME,NAMES...>() != sizeof...(NAMES));

    //! \brief find index of name (size if not found); uses a linear search
    template<string_literal NAME>
      static constexpr size_t find_name = named<T,NAMES...>::template _find_name<0,NAME,NAMES...>();
    
    //! \brief size (number of names/elements in value_type)
    static constexpr size_t size = sizeof...(NAMES);
    
    //! \brief Forwarding constructor
    template<typename... Ts>
      constexpr named(Ts&&...ts) : T{std::forward<Ts>(ts)...} {}

    //! \brief Reshuffle constructor (based on matching names)
    template<typename U, string_literal... UNAMES> requires (sizeof...(NAMES) == sizeof...(UNAMES))
      constexpr named(named<U, UNAMES...>&& src) : named(get<NAMES>(std::forward<decltype(src)>(src))...) { }

    //! \brief Reshuffle constructor (based on matching names)
    template<typename U, string_literal... UNAMES> requires (sizeof...(NAMES) == sizeof...(UNAMES))
      constexpr named(const named<U, UNAMES...>& src) : named(get<NAMES>(src)...) { }

    //! \brief reshuffle assignment
    template<typename U, string_literal... UNAMES> requires (sizeof...(NAMES) == sizeof...(UNAMES))
      named operator=(const named<U,UNAMES...>& src)
    {
      ((this->template get<UNAMES>() = src. template get<UNAMES>()), ...);
      return *this;
    }

    //! \brief unnamed assignment
    template<typename... Ts>  requires (sizeof...(NAMES) == sizeof...(Ts))
      named operator=(const std::tuple<Ts...>& src)
    {
      values() = src;
      return *this;
    }
    
    //! @{ \names Querry/cast values from the underlying container
    constexpr T& values(void) { return *this; }
    constexpr const T& values(void) const { return *this; }

    constexpr operator T(void) { return values(); }
    //! @}
    
    //! @{ \name 'get' by name or index
    template<size_t IDX> constexpr decltype(auto) get(void) { return std::get<IDX>(values()); }
    template<size_t IDX> constexpr decltype(auto) get(void) const { return std::get<IDX>(values()); }
    
    template<string_literal NAME, string_literal... SUBNAME> constexpr decltype(auto) get(void)
    {
      // find name
      constexpr size_t idx = _find_name<0,NAME,NAMES...>();
      static_assert( idx < sizeof...(NAMES), "named structure does not contain requested name.");

      // retreive value; recurse if necessary
      if constexpr (sizeof...(SUBNAME) == 0) return get<idx>();
      else return get<idx>().template get<SUBNAME...>();
    }
    
    template<string_literal NAME, string_literal... SUBNAME> constexpr decltype(auto) get(void) const
    {
      // find name
      constexpr size_t idx = _find_name<0,NAME,NAMES...>();
      static_assert( idx < sizeof...(NAMES), "named structure does not contain requested name.");

      // retreive value; recurse if necessary
      if constexpr (sizeof...(SUBNAME) == 0) return get<idx>();
      else return get<idx>().template get<SUBNAME...>();
    }
    //! @}

    //! \brief ostream
    friend std::ostream& operator<<(std::ostream& s, const named& n)
    {
      s << "(";
      CONSTFOR(idx, sizeof...(NAMES),
      {
        if constexpr (idx != 0) s << ", ";

        // print name
        s << n.template name<idx> << " = ";
          
        // add "" if a string type
        if constexpr (bbm::is_string_type_v< std::tuple_element_t<idx, named> >) s << std::string("\"") + std::get<idx>(n) << std::string("\"");

        // otherwise print value
        else s << std::get<idx>(n);
      });
      s << ")";
      return s;
    }
    
  private:
    template<size_t IDX, string_literal NAME, string_literal N, string_literal... Ns>
      static constexpr size_t _find_name(void)
    {
      if constexpr (NAME == N) return IDX;
      else if constexpr (sizeof...(Ns) != 0) return _find_name<IDX+1, NAME, Ns...>();
      else return sizeof...(NAMES);
    }
  };


  /**********************************************************************/
  /*! @{ \name Type traits
   **********************************************************************/
  template<typename T> struct is_named : std::false_type {};
  template<typename T, string_literal... NAMES> struct is_named<named<T,NAMES...>> : std::true_type {};

  template<typename T>
    constexpr bool is_named_v = is_named<std::decay_t<T>>::value;
  //! @}

  /**********************************************************************/
  /*! @{ \name anonymize trait: remove the names
   **********************************************************************/
  template<typename T>
    constexpr decltype(auto) anonymize_v(T&& t)
  {
    if constexpr (is_named_v<T>) return t.values();
    else return std::forward<T>(t);
  }

  template<typename T>
    using anonymize_t = std::decay_t<decltype(anonymize_v(std::declval<T>()))>;
  //! @}

  /**********************************************************************/
  /*! @{ \name named_equivalence trait: do two types have the same set of names?
   **********************************************************************/
  namespace detail {
    template<typename U, typename T> struct include_names_from : std::false_type {};
    template<typename U, typename T, string_literal... NAMES> requires is_named_v<U>
      struct include_names_from<U, named<T,NAMES...>>: std::bool_constant< (U::template has_name<NAMES> && ...) > {};
  }
  
  template<typename U, typename V> struct named_equivalence : std::false_type {};
  template<typename U, typename V> requires is_named_v<U> && is_named_v<V>
    struct named_equivalence<U,V> : std::conjunction< bbm::detail::include_names_from<std::decay_t<U>,std::decay_t<V>>, bbm::detail::include_names_from<std::decay_t<V>,std::decay_t<U>> > {};

  template<typename U, typename V>
    static constexpr bool named_equivalence_v = named_equivalence<U,V>::value;
  //! @}

  /**********************************************************************/
  /*! \brief Make a named of a gettable type (with size == #NAMES); renames if
      the type is a named container.
   **********************************************************************/
  template<string_literal... NAMES, typename T> requires (sizeof...(NAMES) == 0) || (concepts::gettable<T> && (sizeof...(NAMES) == std::tuple_size_v<std::decay_t<T>>))
    constexpr named<anonymize_t<T>, NAMES...> make_named(T&& t) { return named<anonymize_t<T>,NAMES...>{anonymize_v(std::forward<T>(t))}; }

  /**********************************************************************/
  /*! \brief Make a named tuple from a list of arguments (number of arguments == #NAMES)
   **********************************************************************/
  template<string_literal... NAMES, typename... Ts> requires (sizeof...(NAMES) == sizeof...(Ts))
    constexpr auto make_named(Ts&&... ts) { return make_named<NAMES...>(std::make_tuple(std::forward<Ts>(ts)...)); }
  
  /**********************************************************************/
  /*! \brief Pick a subset/reshuffle a named container T and return as a named tuple
   **********************************************************************/
  template<string_literal... NAMES, typename T> requires is_named_v<T> 
    constexpr auto pick(T&& t) { return make_named<NAMES...>(get<NAMES>(std::forward<T>(t))...); }

  /**********************************************************************/
  /*! \brief Tie by name
   **********************************************************************/
  template<string_literal... NAMES, typename... Ts> requires (sizeof...(NAMES) == sizeof...(Ts))
    named<std::tuple<Ts&...>, NAMES...> tie(Ts&&... src) { return std::tie(std::forward<Ts>(src)...); }

  /**********************************************************************/
  /*! @{ \name Get by name
   **********************************************************************/
  template<string_literal NAME, string_literal... SUBNAME, typename T> requires is_named_v<T>
    inline constexpr decltype(auto) get(T&& src) { return src.template get<NAME, SUBNAME...>(); }

  template<string_literal NAME, string_literal... SUBNAME, typename T> requires is_named_v<T>
    inline constexpr decltype(auto) get(const T& src) { return src.template get<NAME, SUBNAME...>(); }
  //! @}
      
} // end bbm namespace

namespace std {
  
  //! \brief tuple_size specialization (to support structured binding)
  template<typename T, bbm::string_literal... NAMES>
    struct tuple_size<bbm::named<T,NAMES...>> : tuple_size<T> {};

  //! \brief tuple_element specialization (to support structured binding)
  template<size_t idx, typename T, bbm::string_literal... NAMES>
    struct tuple_element<idx, bbm::named<T,NAMES...>> : tuple_element<idx, T> {};

}

///////////////////
// Include utils //
///////////////////
#include "named_util.h"

#endif /* _BBM_NAMED_H_ */
