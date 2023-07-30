#ifndef _BBM_OPTION_H_
#define _BBM_OPTION_H_

#include <map>
#include <set>
#include <string>
#include <stdexcept>
#include <iostream>

#include "util/type_traits.h"

/************************************************************************/
/*! \file option.h
  \brief Simple command line option parser
*************************************************************************/

namespace bbm {

  //! \brief map for storing options (key == string, value == string)
  using option_map = std::map<std::string, std::string>;

  //! \brief Parse commandline argc and argv C strings and return an 'option_map'.
  option_map parse_options(int argc, char** argv)
  {
    option_map m;
    
    for(size_t idx = 1; idx < size_t(argc); ++idx)
    {
      std::string opt (argv[idx]);
      auto split_eq = opt.find("=");

      std::string key = opt.substr(0, split_eq);
      std::string value = (split_eq < opt.size()-1) ? opt.substr(split_eq+1) : "";

      m[key] = value;
    }

    return m;
  }

  //! \brief Validate options: check that 'opt' only contains 'keywords'.
  //! \returns first invalid encountered keyword
  template<typename... Str> requires (is_string_type_v<Str> && ...)
    std::string validate_option_keywords(const option_map& opt, Str&&... keyword)
  {
    // create set of keywords
    std::set<std::string> keyset;
    (keyset.insert(keyword), ...);

    // check if each keyword in opt is in keyset
    for(auto&& o : opt)
      if(! keyset.contains(o.first) ) return o.first;

    // Done.
    return std::string();
  }

  //! @{ \name Option convertors (see get method)
  using string_option = decltype([](const std::string& val) { return val; });
  using bool_option = decltype([](const std::string& val) { return (val != "false" && val != "0" && val != "off"); });
  using float_option = decltype([](const std::string& val) { return atof(val.c_str()); });
  using int_option = decltype([](const std::string& val) { return atoi(val.c_str()); });

  template<typename CONVERT>
    using vec3d_option = decltype([](const std::string& val)
  {
    auto p0 = val.find('[');
    auto p1 = val.find(',', p0+1);
    auto p2 = val.find(',', p1+1);
    auto p3 = val.find(']', p2+1);

    if(p0 == val.size() || p1 == val.size() || p2 == val.size() || p3 == val.size() || p0 > p1 || p1 > p2 || p2 > p3)
      throw std::runtime_error("Parse error for '" + val + "'");

    auto v1 = CONVERT()( val.substr(p0+1, p1-p0-1) );
    auto v2 = CONVERT()( val.substr(p1+1, p2-p1-1) );
    auto v3 = CONVERT()( val.substr(p2+1, p3-p2-1) );
    
    return vec3d<decltype(v1)>(v1, v2, v3);
  });
  //! @}
  
  //! \brief get an option with 'key' from the option_map 'm'. If not found, return a 'default_value'.  The 'value' is passed through an interpreter to convert into a target type.
  template<typename CONVERT=bool_option>
    std::invoke_result_t<CONVERT, std::string> get(const option_map& m, const std::string& key, const std::invoke_result_t<CONVERT,std::string>& default_value=std::invoke_result_t<CONVERT,std::string>{})
  {
    if(m.contains(key)) return CONVERT()( m.at(key) );
    else return default_value;
  }

} // end bbm namespace

#endif /* _BBM_OPTION_H_ */
