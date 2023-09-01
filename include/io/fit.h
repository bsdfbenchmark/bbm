#ifndef _BBM_FIT_H_
#define _BBM_FIT_H_

#include <map>
#include <fstream>

#include "util/string_util.h"
#include "bbm/bsdf_ptr.h"
#include "bbm/bsdf_import.h"

/************************************************************************/
/*! \file fit.h
  \brief read and write BSDF fits to a 'fit' file.

  FIT file format is a text file where:

  + a comment starts with '#', and the remainder of the line is ignored.

  + a valid BSDF start with a 'key' identifier string, followed by '='
  followed by the BSDF in text format (e.g., as produced by bbm::toString).

*************************************************************************/

namespace bbm {
  namespace io {

    /********************************************************************/
    /*! \brief Import a 'fit' file

      \param filename = FIT filename to read from
      \param data = std::map<std::string, bsdf_ptr> to store the fit data to

    *********************************************************************/
    template<typename CONF> requires concepts::config<CONF>
      void importFIT(const std::string& filename, std::map<std::string, bsdf_ptr<CONF>>& data)
    {
      // open
      std::ifstream ifs(filename.c_str());

      // read each line and add to map
      for(std::string line; std::getline(ifs, line);)
      {
        // remove comment
        line =bbm::string::remove_comment(line, "#");
        auto [key, bsdf] = bbm::string::split_eq(line);
        if(!key.empty())
          data.emplace(key, bbm::bsdf_import<CONF>(bsdf));
      }
      
      // Done.
    }

    /********************************************************************/
    /*! \brief Export a 'fit' file

      \param filename = FIT filename to write to
      \param data = std::map<std::string, bsdf_ptr> of fitted data
      \param comment = string of comment to add to the start of the fit file (ignored if empty)

    *********************************************************************/
    template<typename CONF> requires concepts::config<CONF>
      void exportFIT(const std::string& filename, const std::map<std::string, bsdf_ptr<CONF>>& data, const std::string& comment)
    {
      // open
      std::ofstream ofs(filename.c_str());

      // write comments
      std::stringstream ss(comment);
      for(std::string line; std::getline(ss, line);)
        ofs << "# "  << line << std::endl;

      // write each bsdf in data
      for(const auto& bsdf : data)
        ofs << bsdf.first << " = " << bbm::toString(bsdf.second) << std::endl;

      // Done.
    }
    
  } // end io namespace
} // end bbm namespace

#endif /* _BBM_FIT_H_ */
