#ifndef _BBM_STRING_CONVERT_H_
#define _BBM_STRING_CONVERT_H_

#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>

#include "concepts/named.h"
#include "concepts/reflection.h"
#include "concepts/stringconvert.h"

#include "util/string_util.h"
#include "util/typestring.h"
#include "util/reflection.h"
#include "util/constfor.h"
#include "util/tuple.h"
#include "util/vector_util.h"

#include "core/error.h"
#include "core/args.h"


/************************************************************************/
/*! \file stringconvert.h

  \brief Convert a string to an object and vice versa
*************************************************************************/

namespace bbm {

  namespace detail {
    //! \brief Helper concept: check if T has a typedef contructor_args_t and
    //!        it points to a bbm::args
    template<typename T> concept has_constructor_args_t = bbm::concepts::trait_wrapper<typename std::decay_t<T>::constructor_args_t, bbm::is_args>;
  }
  
  /**********************************************************************/
  /*! \brief Default converter class; should be specialized.  Has a fall
      through for types with reflection; if not it will throw a compile error
      when trying to call the static to/fromString methods.
   **********************************************************************/
  template<typename T>
      struct string_converter
   {
     /******************************************************************/
     /*! \brief convert a string to a type T

       If T has a constructor_args_t, then use this to convert from a string.
       Otherwise, if T supports reflection AND a trivial constructor, then use
       reflection to set the attributes.  In both cases, if T meets
       concepts::named then the name must match the keyword. If T supports
       enumerate reflection, then use this.

       Note: these cases are defined in the base case for string_converter
       such that they can be overwritten by type specific specializations.
     ******************************************************************/
     template<typename = T> requires ((concepts::reflection::attributes<T> && std::constructible_from<T>) || bbm::detail::has_constructor_args_t<T>) || concepts::reflection::enumerate<T>
       static inline T fromString(const std::string& str)
     {
       // check name
       if constexpr (concepts::named<T>)
       {
         auto [key, value] = bbm::string::get_keyword(str);

         if( std::string(std::decay_t<T>::name) != key ) throw std::invalid_argument(std::string("BBM: mismatched object name ") + key + ", expected: " + std::string(std::decay_t<T>::name));

         // if has constructor_args_t; get values and make T
         if constexpr (bbm::detail::has_constructor_args_t<T>)
         {
           auto args = bbm::fromString<typename std::decay_t<T>::constructor_args_t>(value);
           return std::make_from_tuple<T>(args.values());
         }

         // if has attributes, then copying values
         else
         {
           T obj;
         
           // copy values
           using str_type = value_copy_named_t< bbm::reflection::attributes_t<T> >;
           bbm::reflection::attributes(obj) = bbm::fromString<str_type>(value);
         
           return obj;
         }
       }

       // if has enumeration reflection, then search the matching name
       else if constexpr (concepts::reflection::enumerate<T>)
       {
         return linear_search_named(str, bbm::reflection::enum_v<T>, []<size_t IDX, typename ENUM>(const std::string& value, ENUM&& enum_v) -> T
         {
           if constexpr (IDX == std::decay_t<ENUM>::size)
             throw std::invalid_argument(std::string("BBM: name '") + value + "' not found in enumeration: " + std::string(typestring<std::decay_t<T>>));
           else return std::get<IDX>(enum_v);
         });
       }
       
       // Done.
     }


     /*******************************************************************/
     /*! \brief convert an object to a type T

       If T support reflection, then convert the attributes to a string.  If T
       also meets concepts::named, then prefix the name.
     ********************************************************************/
     template<typename = T> requires concepts::reflection::attributes<T> || concepts::reflection::enumerate<T>
       static inline std::string toString(const T& obj)
     {
       std::string result;

       // add name
       if constexpr (concepts::named<T>) result = std::string(std::decay_t<T>::name);

       // add attributes if attribute reflection
       if constexpr (concepts::reflection::attributes<T>)
         result += bbm::toString(bbm::reflection::attributes(obj));

       // add names if enumerate
       if constexpr (concepts::reflection::enumerate<T>)
       {
         CONSTFOR(IDX, bbm::reflection::enum_v<T>.size,
         {
           if(obj == std::get<IDX>(bbm::reflection::enum_v<T>))
             result += std::string( bbm::reflection::enum_v<T>.template name<IDX> );
         });
       }
       
       // done.
       return result;
     }
   };


  /**********************************************************************/
  /*! @{ \name Scalar convertors
   **********************************************************************/
  template<> struct string_converter<float>
  {
    static inline float fromString(const std::string& str) { return std::stof(str); }
    static inline std::string toString(float v) { std::stringstream ss; ss << v; return ss.str(); } // prefer this over to_string (i.e., 1 vs 1.0)
  };

  template<> struct string_converter<double>
  {
    static inline double fromString(const std::string& str) { return std::stod(str); }
    static inline std::string toString(double v) { std::stringstream ss; ss << v; return ss.str(); } // prefer this over to_string
  };

  template<> struct string_converter<long double>
  {
    static inline long double fromString(const std::string& str) { return std::stold(str); }
    static inline std::string toString(long double v) { std::stringstream ss; ss << v; return ss.str(); } // prefer this over to_string
  };

  template<> struct string_converter<int>
  {
    static inline int fromString(const std::string& str) { return std::stoi(str); }
    static inline std::string toString(int v) { return std::to_string(v); }
  };

  template<> struct string_converter<long>
  {
    static inline long fromString(const std::string& str) { return std::stol(str); }
    static inline std::string toString(long v) { return std::to_string(v); }
  };

  template<> struct string_converter<long long>
  {
    static inline long long fromString(const std::string& str) { return std::stoll(str); }
    static inline std::string toString(long long v) { return std::to_string(v); }
  };

  template<> struct string_converter<unsigned int>
  {
    static inline unsigned int fromString(const std::string& str) { return std::stoul(str); }
    static inline std::string toString(unsigned int v) { return std::to_string(v); }
  };

  template<> struct string_converter<unsigned long>
  {
    static inline unsigned long fromString(const std::string& str) { return std::stoul(str); }
    static inline std::string toString(unsigned long v) { return std::to_string(v); }
  };

  template<> struct string_converter<unsigned long long>
  {
    static inline unsigned long long fromString(const std::string& str) { return std::stoull(str); }
    static inline std::string toString(unsigned long long v) { return std::to_string(v); }
  };
  //! @}

  /**********************************************************************/
  /*! \brief bool conversion
   **********************************************************************/
  template<>
    struct string_converter<bool>
  {
    static inline bool fromString(const std::string& str)
    {
      // cast to lower case to match different capitalizations of 'true' and 'false'
      std::string newStr(str);
      std::transform(str.begin(), str.end(), newStr.begin(), [](unsigned char c) { return std::tolower(c); });

      // determine of true/1 of false/0
      bool True = (newStr == "true") || (newStr == "1");
      bool False = (newStr == "false") || (newStr == "0");

      // If neither true nor false, then throw error
      if(!True && !False) throw std::invalid_argument(std::string("BBM: cannot convert to bool from: ") + str);

      // Done.
      return True;
    }

    static inline std::string toString(bool b)
    {
      if(b) return std::string("True");
      else return std::string("False");
    }
  };

  /**********************************************************************/
  /*! \brief string conversion
   **********************************************************************/
  template<>
    struct string_converter<std::string>
  {
    static inline std::string fromString(const std::string& str)
    {
      // remove quotes
      const char* quotes = "\"' \r\n\t\v";
      size_t s = str.find_first_not_of(quotes);
      size_t e = str.find_last_not_of(quotes);

      // throw error if no quotes found
      if(s > e) throw std::invalid_argument(std::string("BBM: connect convert to string; unbalanced quotes in: ") + str);

      // remove qoutes
      return str.substr(s, e-s+1);
    }

    static inline std::string toString(const std::string& str)
    {
      return std::string("\"") + str + std::string("\"");
    }
  };

  /**********************************************************************/
  /*! \brief string_literal conversion (read only)
   **********************************************************************/
  template<size_t N>
    struct string_converter<string_literal<N>>
  {
    static inline std::string toString(const string_literal<N>& str)
    {
      return std::string(str);
    }
  };
  
  /**********************************************************************/
  /*! \brief char conversion
   **********************************************************************/
  template<typename T> requires bbm::concepts::same_as_any<T, char, unsigned char, signed char>
    struct string_converter<T>
  {
    static inline T fromString(const std::string& str)
    {
      // char must be surrounded by ' '.
      if(str.size() != 3 || str[0] != '\'' || str[2] != '\'') throw std::invalid_argument(std::string("BBM: cannot convert to " + std::string(typestring<T>) + " from: " + str));

      // Done.
      else return str[1];
    }

    static inline std::string toString(const T& c)
    {
      return std::string("'") + c + std::string("'");
    }
  };


  /**********************************************************************/
  /*! \brief convert a bbm vector
   **********************************************************************/
  template<typename T>
    struct string_converter<bbm::vector<T>>
  {
    static inline bbm::vector<T> fromString(const std::string& str)
    {
      std::vector<T> result;

      // split in arguments
      auto args = bbm::string::split_args(bbm::string::remove_brackets(str));

      // try to convert each argument to T
      try {
        for(auto& a : args) result.push_back( bbm::fromString<T>(a) );
      } catch(...) {
        throw std::invalid_argument(std::string("BBM: cannot convert to vector<") + std::string(typestring<T>) + "> from: " + str); 
      }

      // Done.
      return result;
    }

    static inline std::string toString(const bbm::vector<T>& vec)
    {
      std::string result("(");
      for(auto itr=vec.begin(); itr != vec.end(); ++itr)
      {
        if(itr != vec.begin()) result += ", ";
        result += bbm::toString(*itr);
      }
      result += ")";
      return result;
    }
  };

  /**********************************************************************/
  /*! \brief convert a std::vector (forward to bbm::vector)
   **********************************************************************/
  template<typename T> struct string_converter<std::vector<T>> : public string_converter<bbm::vector<T>> {};
  
  /**********************************************************************/
  /*! \brief convert to a fixed length array
   **********************************************************************/
  template<typename T, size_t N>
    struct string_converter<std::array<T, N>>
  {
    static inline std::array<T,N> fromString(const std::string& str)
    {
      std::array<T, N> arr;

      // try to get vector
      bbm::vector<T> vec;
      try {
        vec = bbm::fromString<bbm::vector<T>>(str);
      } catch(...) {

      // alternative: try to get single value
      try {
        vec.push_back( bbm::fromString<T>(str) );
      } catch(...) {
        
        // failure in both cases
        throw std::invalid_argument(std::string("BBM: cannot convert to std::array<") + std::string(typestring<T>) + ", " + std::to_string(N) + "> from: " + str);
      }}

      // copy from vector
      if(vec.size() == N) std::copy(vec.begin(), vec.end(), arr.begin());
      else if(vec.size() == 1) std::fill(arr.begin(), arr.end(), vec[0]);
      else throw std::invalid_argument(std::string("BBM: too few arguments to convert to std::array<") + std::string(typestring<T>) + ", " + std::to_string(N) + ">. Found " + std::to_string(vec.size()) + " argument in: " + str);
      
      // Done.
      return arr;
    }

    static inline std::string toString(const std::array<T,N>& arr)
    {
      std::string result("[");
      for(auto itr=arr.begin(); itr != arr.end(); ++itr)
      {
        if(itr != arr.begin()) result += ", ";
        result += bbm::toString(*itr);
      }
      result += "]";
      return result;
    }
  };

  /**********************************************************************/
  /*! \brief convert tuple
   **********************************************************************/
  template<typename... Ts>
    struct string_converter<std::tuple<Ts...>>
  {
    static inline std::tuple<Ts...> fromString(const std::string& str)
    {
      auto args = bbm::string::split_args(bbm::string::remove_brackets(str));

      // check length
      if(args.size() != sizeof...(Ts)) throw std::invalid_argument("BBM: too few arguments to convert to " + std::string(typestring<std::tuple<Ts...>>) + ", expected " + std::to_string(sizeof...(Ts)) + " but found " + std::to_string(args.size()) + " in: " + str);

      // create tuple
      auto args_to_tup = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        return std::make_tuple( bbm::fromString<std::tuple_element_t<IDX, std::tuple<Ts...>>>(args[IDX])... );
      };

      // try conversion
      try {
        return args_to_tup(std::make_index_sequence<sizeof...(Ts)>{});
      } catch(...) {
        throw std::invalid_argument("BBM: cannot convert to " + std::string(typestring<std::tuple<Ts...>>) + " from: " + str);
      }

      // Done.
    }

    static inline std::string toString(const std::tuple<Ts...>& tup)
    {
      std::string result("(");
      CONSTFOR(idx, sizeof...(Ts),
      {
        if constexpr (idx != 0) result += ", ";
        result += bbm::toString(std::get<idx>(tup));
      });
      result += ")";
      return result;
    }
  };

  /**********************************************************************/
  /*! \brief convert a named tuple
    *********************************************************************/
  template<typename TUP, string_literal... NAMES>
    struct string_converter<named<TUP, NAMES...>>
  {
    using type = named<TUP, NAMES...>;
    static constexpr size_t size = sizeof...(NAMES);
    
    static inline type fromString(const std::string& str)
    {
      auto args = bbm::string::split_args(bbm::string::remove_brackets(str));

      // check length
      if(args.size() != size) throw std::invalid_argument("BBM: too few arguments to convert to " + std::string(typestring<type>) + ", expected " + std::to_string(size) + " but found " + std::to_string(args.size()) + " in: " + str);

      // create name list
      std::array<std::string, size> names;
      CONSTFOR(idx, size, { names[idx] = type::template name<idx>; });

      // create key/value list from args
      std::array<std::pair<std::string, std::string>, size> keyval;
      for(size_t idx=0; idx != size; ++idx)
        keyval[idx] = bbm::string::split_eq(args[idx]);

      // find keyword position
      auto get_permutation = [&](size_t idx)
      {
        auto key = keyval[idx].first;

        // if no keyword
        if(key.empty()) return idx;

        // else, find keyword (slow linear search; assumes only a few names)
        auto match = std::find(std::begin(names), std::end(names), key);

        // throw error if not found
        if(match == std::end(names)) throw std::invalid_argument("BBM: cannot convert to "  + std::string(typestring<type>) + ": invalid name: " + key + " in " + str);

        return size_t(std::distance(std::begin(names), match));
      };

      // create index permuation 
      std::array<size_t, size> perm;
      std::array<bool, size> used; std::fill(std::begin(used), std::end(used), false);

      for(size_t idx=0; idx < size; ++idx)
      {
        size_t match = get_permutation(idx);
        perm[idx] = match;
        used[match] = true;
      }

      // check if each element is assigned
      for(size_t idx=0; idx < size; ++idx)
        if(!used[idx]) throw std::invalid_argument("BBM: cannot convert to "  + std::string(typestring<type>) + ": " + std::to_string(idx) + " element (" + names[idx] + ") not assigned.");
      
      // create named tuple
      auto create_named = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        return make_named<NAMES...>( bbm::fromString<std::tuple_element_t<IDX, type>>(keyval[perm[IDX]].second)... );
      };

      // Done.
      return create_named( std::make_index_sequence<size>{} );
    }
    
    static inline std::string toString(const named<TUP, NAMES...>& obj)
    {
      std::string result("(");
      CONSTFOR(idx, sizeof...(NAMES),
      {
        if constexpr (idx != 0) result += ", ";

        result += std::string(std::decay_t<decltype(obj)>::template name<idx>);
        result += " = ";
        result += bbm::toString(std::get<idx>(obj));
      });
      result += ")";
      return result;
    }
  };

  /**********************************************************************/
  /*! \brief convert args<...>
   **********************************************************************/
  template<typename ARGS> requires is_args_v<ARGS>
    struct string_converter<ARGS>
  {
    static inline std::string toString(const ARGS& args)
    {
      return bbm::toString(args.values());
    }

    static inline ARGS fromString(const std::string& str)
    {
      auto args = bbm::string::split_args(bbm::string::remove_brackets(str));

      // check if size of args does not exceed the number of ARG in ARGS
      if(args.size() > ARGS::size) throw std::invalid_argument(std::string("BBM: expected at most ") + std::to_string(ARGS::size) + " arguments, found " + std::to_string(args.size()) + " in: " + str);
      
      // create run-time searchable list of ARG names
      std::set<std::string> arg_names;
      CONSTFOR(idx, ARGS::size,
      {
        arg_names.insert( std::string(ARGS::template name<idx>) );
      });

      // extract values and names of arguments in string
      std::map<std::string, size_t> name_map;
      std::vector<bool> named_arg;
      std::vector<std::string> values;
      
      for(size_t idx=0; idx != args.size(); ++idx)
      {
        auto [key, value] = bbm::string::split_eq(args[idx]);

        // if named, check if valid name and add to map
        if(key != "")
        {
          named_arg.push_back(true);
          if(arg_names.contains(key)) name_map[key] = idx;
          else throw std::invalid_argument(std::string("BBM: invalid argument name: ") + key + "(" + value + ") in: " + str);
        }
        else named_arg.push_back(false);
        
        // store value
        values.push_back(value);
      }

      // retrieve bbm::arg from string or set to default value
      auto retrieve_arg = [&]<size_t IDX>(void)
      {
        using arg_t = std::decay_t<typename ARGS::template type<IDX>>;
        using type = std::decay_t<typename arg_t::type>;

        // does ARG::name exist in args?
        auto name_itr = name_map.find(std::string(arg_t::name));
        if(name_itr != name_map.end()) return arg_t( bbm::fromString<type>(values[name_itr->second]) );

        // else check position
        if(IDX < values.size() && !named_arg[IDX]) return arg_t( bbm::fromString<type>(values[IDX]) );

        // else check if default value
        if constexpr (std::is_constructible_v<arg_t>) return arg_t();

        // else fail
        else throw std::invalid_argument(std::string("BBM: unable to find argument ") + std::string(arg_t::name) + " in: " + str);
      };

      // for each ARG in ARGS, get value
      auto create_args = [&]<size_t... IDX>(std::index_sequence<IDX...>)
      {
        return ARGS( retrieve_arg.template operator()<IDX>()... );
      };

      // Done.
      return create_args(std::make_index_sequence<ARGS::size>{});
    }
  };

  
  /**********************************************************************/
  /*! \brief fromString alias
   **********************************************************************/
  template<typename T> requires concepts::from_stringconvert<T>
    inline T fromString(const std::string& str)
  {
    if constexpr (concepts::has_fromString<T>) return std::decay_t<T>::fromString(str);
    else return string_converter<std::decay_t<T>>::fromString(str);
  }

  /**********************************************************************/
  /*! \brief toString alias
   **********************************************************************/
  template<typename T> requires concepts::to_stringconvert<T>
    inline std::string toString(const T& obj)
  {
    if constexpr (concepts::has_toString<T>) return obj.toString();
    else return string_converter<std::decay_t<T>>::toString(obj);
  }
  
  
} // end namespace bbm

#endif /* _BBM_STRING_CONVERT_H_ */
