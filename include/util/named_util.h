#ifndef _BBM_NAMED_UTIL_H_
#define _BBM_NAMED_UTIL_H_

#include "util/named.h"

/************************************************************************/
/*! \file named_util.h
  \brief Additional convenience methods for named tuples.
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief value copy a named tuple

    \param src = named tuple, possibly with references
    \returns a copy of the named tuple without references
  ***********************************************************************/
  template<typename TUP, string_literal... NAMES> requires bbm::is_tuple_v<TUP>
    inline constexpr auto value_copy_named(const named<TUP, NAMES...>& src)
  {
    return make_named<NAMES...>( value_copy_tuple(src.values()) );
  }

  //! \brief type of value copying a named typle
  template<typename T> requires is_named_v<T> && is_tuple_v<typename T::value_type>
    using value_copy_named_t = decltype( value_copy_named(std::declval<T>()) );

  
  /**********************************************************************/
  /*! \brief cat named types

    named_cat( named<std::tuple<...>, "A", "B">{a,b}, named<std::tuple<...>, "C">{c} )

    yields

    named<std::tuple<...>, "A", B", "C">{a,b,c}
   **********************************************************************/
  template<typename... T> requires (is_named_v<T> && ...)
    inline constexpr auto named_cat(T&&... t)
  {
    // helper lambda; use tuple_cat on names and values
    auto merge = [&]<size_t... IDX>(std::index_sequence<IDX...>)
    {
      constexpr auto names = std::tuple_cat( std::decay_t<T>::names...);
      return make_named< std::get<IDX>(names)... >( std::tuple_cat(t.values()...) );
    };

    // if empty return default; otherwise call helper lambda
    if constexpr (sizeof...(T) == 0) return named<std::tuple<>>();
    else return merge(std::make_index_sequence< (std::tuple_size_v<std::decay_t<T>> + ...) >{});
  }

  //! \brief type of concatting multiple named tuples.
  template<typename... T> requires (is_named_v<T> && ...)
    using named_cat_t = decltype( named_cat(std::declval<T>()...) );


  /**********************************************************************/
  /*! \brief get a subset of a named tuple

    \tparam START = index of first element
    \tparam COUNT =  number of elements
    \param named = named tuple
    \returns named tuple with elements [START, START+1, ..., START+COUNT-1
  ***********************************************************************/
  template<size_t START, size_t COUNT, typename NAMED> requires is_named_v<NAMED> && ((START+COUNT) <= std::decay_t<NAMED>::size)
    inline constexpr auto subnamed(NAMED&& named)
  {
    // zero length case or beyond end
    if constexpr (COUNT == 0) return named<std::tuple<>{};

    // otherwise select subset
    else
    {
      // helper lambda to extract the elements
      auto extract = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        return bbm::named<std::tuple< typename std::tuple_element_t<START+IDX, std::decay_t<NAMED>>... >, std::decay_t<NAMED>::template name<START+IDX>...>(std::get<START+IDX>(std::forward<NAMED>(named))...);
      };
    
      return extract(std::make_index_sequence<COUNT>{});
    }
  }

  //! \brief type of subnamed
  template<size_t START, size_t COUNT, typename NAMED> requires is_named_v<NAMED> && ((START+COUNT) <= std::decay_t<NAMED>::size)
    using subnamed_t = decltype( subnamed<START,COUNT,NAMED>( std::declval<NAMED>() ) );


  /**********************************************************************/
  /*! \brief prefix names in type

    prefix_names<"BLA_", named<std::tuple<...>, "A", "B">{a,b}

    yields

    named<std::tuple<...>, "BLA_A", "BLA_B">{a,b}
    *********************************************************************/
  template<string_literal PREFIX, typename T, string_literal... NAMES>
    inline constexpr auto prefix_names(named<T, NAMES...> t)
  {
    return static_cast<named<T, (PREFIX+NAMES)...>>(t);
  }

  //! \brief type of named tuple with pre-fixed name.
  template<string_literal PREFIX, typename T>  requires is_named_v<T>
    using prefix_names_t = decltype(prefix_names<PREFIX>(std::declval<T>()));

  
  /**********************************************************************/
  /*! \brief postfix names in type

    postfix_names<"_BLA", named<std::tuple<...>, "A", "B">{a,b}

    yields

    named<std::tuple<...>, "A_BLA", "B_BLA">{a,b}
  *********************************************************************/
  template<string_literal POSTFIX, typename T, string_literal... NAMES>
    inline constexpr auto postfix_names(named<T, NAMES...> t)
  {
    return static_cast<named<T, (NAMES + POSTFIX)...>>(t);
  }

  //! \brief type of named tuple with post-fixed name.
  template<string_literal POSTFIX, typename T>  requires is_named_v<T>
    using postfix_names_t = decltype(post_names<POSTFIX>(std::declval<T>()));

  
  /*** Named flatten implementation details ***/
  namespace detail {

    template<string_literal PREFIX, string_literal SEP, bool cat_names, typename T>
      inline constexpr auto named_flatten(T&& t)
    {
      // base case: T is not named
      if constexpr (!is_named_v<T>) { return make_named<PREFIX>(std::forward_as_tuple(t)); }

      // else: recurse
      else
      {
        auto flatten = [&]<size_t... IDX>(std::index_sequence<IDX...>)
        {
          constexpr auto names = std::decay_t<T>::names;
          return named_cat( (named_flatten<std::get<IDX>(names), SEP, cat_names>(std::get<IDX>(std::forward<T>(t))) )...);
        };

        // cat all
        auto result = flatten(std::make_index_sequence<std::decay_t<T>::size>{});

        // prefix names if requested.
        if constexpr (cat_names && !PREFIX.empty) return prefix_names<PREFIX + SEP>( result );
        else return result;
      }
    }

  } // end detail namespace
  
  /**********************************************************************/
  /*! \brief flatten a named type without merging names

    named_flatten( named< std::tuple<named<std::tuple<float, char>, "A", "B">, int>, "C", "D" > )

    yields

    named<std::tuple<float, char, int>, "A", "B", "D">
   **********************************************************************/
  template<typename T> requires is_named_v<T>
    inline constexpr auto named_flatten(T&& t) { return bbm::detail::named_flatten<"", "", false>(std::forward<T>(t));  }

  /**********************************************************************/
  /*! \brief flatten a named type with merging names

    merge_named_flatten( named< std::tuple<named<std::tuple<float, char>, "A", "B">, int>, "C", "D" > )

    yields

    named<std::tuple<float, char, int>, "C.A", "C.B", "D">

    The default seperator symbol is ".", but this can be changed by passing a
    different symbols as the first template argument.
   **********************************************************************/
  template<string_literal SEP=".", typename T> requires is_named_v<T>
    inline constexpr auto merge_named_flatten(T&& t) { return bbm::detail::named_flatten<"", SEP, true>(std::forward<T>(t));  }


  /*** Implementation detail for is_named_sorted ***/
  namespace detail {
    template<typename NAMED>
      inline constexpr bool is_named_sorted(void)
    {
      auto all_sorted = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        return ((std::decay_t<NAMED>::template name<IDX> < std::decay_t<NAMED>::template name<IDX+1>) && ...);
      };

      if constexpr (std::decay_t<NAMED>::size <= 1) return true;
      else return all_sorted(std::make_index_sequence<std::decay_t<NAMED>::size - 1>{});
    }
  } // end detail namespace
  
  /**********************************************************************/
  /*! \brief true if named tuple is sorted by name
     ********************************************************************/
  template<typename NAMED> requires is_named_v<NAMED>
    static constexpr bool is_named_sorted_v = detail::is_named_sorted<NAMED>();
  

  /*** Implementation details for binary_search_named ***/
  namespace detail {
    template<string_literal NAME, typename NAMED, size_t START=0, size_t END = NAMED::size>
      inline constexpr size_t binary_search_named(void)
    {
      if constexpr (START >= END) return START;
      else
      {
        constexpr size_t IDX = (START + END) / 2;
        if constexpr (IDX == NAMED::size) return IDX;
        else if constexpr (NAMED::template name<IDX> == NAME) return IDX;
        else if constexpr (NAMED::template name<IDX> < NAME) return binary_search_named<NAME, NAMED, IDX+1, END>();
        else return binary_search_named<NAME, NAMED, START, IDX>();
      }
    }
  } // end detail namespace
  
  /**********************************************************************/
  /*! \brief binary search a _sorted_ named tuple

    \tparam NAME = name to search for
    \tparam NAMED = named tuple
    \returns index of best matching name (index of first name that is equal or larger)
  ***********************************************************************/
  template<string_literal NAME, typename NAMED> requires is_named_v<NAMED> && is_named_sorted_v<NAMED>
    static constexpr size_t binary_search_named_v = detail::binary_search_named<NAME, std::decay_t<NAMED>>();
  

  /**********************************************************************/
  /*! \brief sort a named tuple by name using insert-sort.

    \param named = named tuple to sort
    \tparam IDX = start index of elements to sort; recommended default value = 0
    \tparam PARTIAL = partial solution to insert into; recommended default value = empty named tuple
    \returns sorted(NAMED[IDX..END], PARTIAL)

    Details: this is a constexpr recursive method.  It will attempt to insert
    the IDX-th element in PARTIAL, and then recurse to the IDX+1 element,
    until all elements are inserted.
  ************************************************************************/
  template<typename NAMED, size_t IDX=0, typename PARTIAL=named<std::tuple<>>>
    inline constexpr auto sort_named(NAMED&& named, PARTIAL&& partial = PARTIAL{})
  {
    // if all processed, return PARTIAL 'p'
    if constexpr (std::decay_t<NAMED>::size <= IDX) return partial;

    // insert the IDX-th element from 'n' in PARTIAL 'p', and then recurse to insert IDX+1
    else
    {
      // insert
      auto insert = []<string_literal NAME>(auto&& t, auto&& n)
      {
        using N = decltype(n);
        using T = decltype(t);
        constexpr size_t INSERT = binary_search_named_v<NAME, N>;
        if constexpr (INSERT == 0) return named_cat( make_named<NAME>(std::forward<T>(t)), std::forward<N>(n) );
        else if constexpr (INSERT == std::decay_t<N>::size) return named_cat( std::forward<N>(n), make_named<NAME>(std::forward<T>(t)) );
        else return named_cat( subnamed<0, INSERT>(std::forward<N>(n)), make_named<NAME>(std::forward<T>(t)), subnamed<INSERT, std::decay_t<N>::size - INSERT>(std::forward<N>(n)) );
      };
      
      // recurse
      return sort_named<NAMED, IDX+1>( std::forward<NAMED>(named), insert.template operator()< std::decay_t<NAMED>::template name<IDX> >( std::get<IDX>(std::forward<NAMED>(named)), partial) );
    }
  }


  /*** Implementation detail for binary_search_named with strings ***/
  namespace detail {
    /********************************************************************/
    /*! \brief recusively search a sorted named tuple for a name that matches
        a string, and then call a lambda function to process the found element.

        \tparam START = start index in named to search
        \tparam END = end index (+1) in named to search
        \param str = string to match
        \param named = named tuple to seach in
        \param process = lambda to call on found result
        \param context... = additional variables to pass to the process lambda

        See bbm::find_named_sorted for details on the process lambda.
    ***********************************************************************/
    template<size_t START, size_t END, typename NAMED, typename PROCESS, typename... Ts>
      inline auto binary_search_named(const std::string& str, NAMED&& named, PROCESS&& process, Ts&&... context)
    {
      // recursion stopped without finding an exact match; call process with best matching index.
      if constexpr (START >= END) return process.template operator()<START>(str, std::forward<NAMED>(named), std::forward<Ts>(context)...);

      // recurse
      else
      {
        constexpr size_t IDX = (START+END) / 2;
        static constexpr size_t size = std::decay_t<NAMED>::size;
        std::string name(std::decay_t<NAMED>::template name<IDX>);

        // match found (or out of bound) => call process with best matching index
        if(IDX == size || name == str) return process.template operator()<IDX>(str, std::forward<NAMED>(named), std::forward<Ts>(context)...);

        // recurse right
        else if(name < str) return binary_search_named<IDX+1, END>(str, std::forward<NAMED>(named), std::forward<PROCESS>(process), std::forward<Ts>(context)...);

        // recurse left
        else return binary_search_named<START, IDX>(str, std::forward<NAMED>(named), std::forward<PROCESS>(process), std::forward<Ts>(context)...);
      }

      // Done.
    }
  } // end detail namespace

  /**********************************************************************/
  /*! \brief Run-time binary search for a matching name in a named tuple based
      on a string.  The (index of the) found element is forwarded to a
      processing lambda.

      \param str = string of name to find
      \param named = (sorted) named tuple
      \param process = lambda function to process the found element
      \param context... = additional parameters to pass to process
      
    Q&A:
        
    + Why call a processing lambda? Each element in the tuple can potentially
    have a different type. Hence we cannot just return the found result.

    + Why not return an index to the found element? The returned index would
    not be a constexpr, and hence you cannot use it to get the corresponding
    element with std::get.

    + What is the signature of the process lambda: The lambda takes the index
    of the found element as a template parameter (size_t), and it takes the
    following run-time arguments: a 'const std::string&' of the searched for
    string, the named tuple, and all context variables.

    + What if no match is found?  the process lambda is expected to check if
    (index < NAMED::size) and that (str == std::get<IDX>(named))
    
    + What is the goal of the 'context' variables?  To give the processing
    lambda access to any data that it needs to complete its task (including
    variables to store results in).

    + Can process return a result?  Yes, as long as the return type is the
    same regardless of the index of the found element. Consider using std::any
    or context variables if this is not possible.

  ***********************************************************************/
  template<typename NAMED, typename PROCESS, typename... Ts> requires
    is_named_v<NAMED> &&         // must be a named tuple
    is_named_sorted_v<NAMED> &&  // named tuple must be sorted
    requires(PROCESS&& p)        // check the signature of process
  { p.template operator()<size_t(0)>(std::declval<std::string>(), std::declval<NAMED>(), std::declval<Ts>()...); }
    inline auto binary_search_named(const std::string& str, NAMED&& named, PROCESS&& process, Ts&&... context)
  {
    return bbm::detail::binary_search_named<0, std::decay_t<NAMED>::size>(str, std::forward<NAMED>(named), std::forward<PROCESS>(process), std::forward<Ts>(context)...);
  }

    
  /**********************************************************************/
  /*! \brief linear search for unsorted named tuples

    \param str = string to find
    \param named = named tuple to search
    \param process = lambda function to process the found element
    \param context... = additional parameters to pass to process

    Process will be called with IDX == NAMED::size if not found.
    
  ***********************************************************************/
  template<typename NAMED, typename PROCESS, typename... Ts> requires
    is_named_v<NAMED> &&         // must be a named tuple
    requires(PROCESS&& p)        // check the signature of process
  { p.template operator()<size_t(0)>(std::declval<std::string>(), std::declval<NAMED>(), std::declval<Ts>()...); }
    inline auto linear_search_named(const std::string& str, NAMED&& named, PROCESS&& process, Ts&&... context)
  {
    using result_t = decltype( process.template operator()<size_t(0)>(std::declval<std::string>(), std::declval<NAMED>(), std::declval<Ts>()...) );

    // lambda returns void
    if constexpr (std::is_void_v<result_t>)
    {
      bool found = false;
      
      CONSTFOR(IDX, std::decay_t<NAMED>::size,
      {
        if(str == std::string(std::decay_t<NAMED>::template name<IDX>)) process.template operator()<IDX>(str, std::forward<NAMED>(named), std::forward<Ts>(context)...);
      });

      // if not found, call with out-of-bound index.
      if(!found) process.template operator()<std::decay_t<NAMED>::size>(str, std::forward<NAMED>(named), std::forward<Ts>(context)...);

    }

    // else lambda returns a value
    else
    {
      result_t result;
      bool found = false;
      
      CONSTFOR(IDX, std::decay_t<NAMED>::size,
      {
        if(str == std::string(std::decay_t<NAMED>::template name<IDX>))
        {
          result = process.template operator()<IDX>(str, std::forward<NAMED>(named), std::forward<Ts>(context)...);
          found = true;
        }
      });

      // if not found, call with out-of-bound index.
      if(!found) result =  process.template operator()<std::decay_t<NAMED>::size>(str, std::forward<NAMED>(named), std::forward<Ts>(context)...);
      
      return result;
    }

    // Done.
  } 
    
} // end bbm namespace

#endif /* _BBM_NAMED_UTIL_H_ */
