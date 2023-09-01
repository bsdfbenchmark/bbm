#ifndef _BBM_OPTION_H_
#define _BBM_OPTION_H_

#include <map>
#include <string>
#include <stdexcept>

#include "concepts/stringconvert.h"
#include "util/string_util.h"


/************************************************************************/
/*! \file option.h
  \brief Simple command line option parser
*************************************************************************/

namespace bbm {

  struct option_parser
  {
    //! \brief Parse the options from the commandline arguments (argc,argv)
    inline option_parser(int argc, char** argv)
    {
      for(size_t idx = 1; idx < size_t(argc); ++idx)
      {
        auto [key, val] = bbm::string::split_eq(argv[idx]);

        // case 1: key = val
        if(key != "") _map[key] = val;

        // case 2: val (= true)
        else _map[val] = std::string("true");
      }
    }

    //! \brief check keys in the options are in the set keywords.
    //! \returns vector of erroneous keywords in options.
    inline std::vector<std::string> validate(const std::set<std::string>& keywords) const
    {
      std::vector<std::string> err;
      for(auto& key : _map)
        if(!keywords.contains(key.first)) err.push_back(key.first);
      return err;
    }
    
    //! \brief retrieve an option
    template<typename RET>
      inline RET get(const std::string& key) const
    {
      if(_map.contains(key)) return bbm::fromString<RET>( _map.at(key) );
      else throw std::runtime_error(std::string("Missing required option ") + key);
    }
    
    //! \brief retrieve an option
    template<typename RET>
      inline RET get(const std::string& key, const RET& default_value) const
    {
      if(_map.contains(key)) return bbm::fromString<RET>( _map.at(key) );
      else return default_value;
    }
    
  private:
    std::map<std::string, std::string> _map;
  };
  
} // end bbm namespace

#endif /* _BBM_OPTION_H_ */
