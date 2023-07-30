#ifndef _BBM_TYPE_NAME_H_
#define _BBM_TYPE_NAME_H_

#include <string_view>

#if !defined(__clang__) || !defined(__GNUC__) || !defined(__MSC_VER)
#include <typeinfo>
#endif

/***********************************************************************/
/*! \file typestring.h
    \brief produce stringview of type name of a type. Avoids using typeid
    for GCC, MSVC, and CLANG. For other compilers it will fall back to typeid.name()
************************************************************************/

namespace bbm {

  namespace detail {
    template <typename T>
      inline constexpr std::string_view  typestring_impl(void)
    {
#if defined(__clang__)
      constexpr auto prefix = std::string_view{"[T = "};
      constexpr auto suffix = std::string_view{"]"};
      constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(__GNUC__)
      constexpr auto prefix = std::string_view{"with T = "};
      constexpr auto suffix = std::string_view{"; "};
      constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(__MSC_VER)
      constexpr auto prefix = std::string_view{"get_type_name<"};
      constexpr auto suffix = std::string_view{">(void)"};
      constexpr auto function = std::string_view{__FUNCSIG__};
#else
      constexpr auto prefix = std::string_view{""};
      constexpr auto suffix = std::string_view{""};
      auto function = std::string_view( typeid(T{}).name() );
      return function;
#endif

      const auto start = function.find(prefix) + prefix.size();
      const auto end = function.find(suffix);
      const auto size = end - start;
      
      return function.substr(start, size);
    }

  } // end detail namespace

  template<typename T>
    static constexpr std::string_view typestring = bbm::detail::typestring_impl<T>();

  #define toTypestring(...) bbm::typestring<decltype(__VA_ARGS__)>
    
}

#endif /* _BBM_TYPE_NAME_H_ */
