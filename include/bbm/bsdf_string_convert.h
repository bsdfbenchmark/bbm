#ifndef _BBM_BSDF_STRING_CONVERT_H_
#define _BBM_BSDF_STRING_CONVERT_H_

#include "core/stringconvert.h"

#include "bbm/bsdf.h"
#include "bbm/bsdf_ptr.h"
#include "bbm/aggregatebsdf.h"

/************************************************************************/
/*! \file bsdf_string_convert.h

  \brief Specialized conversion between strings and bsdfs not handeled by the
  default string convertion defined in core/stringconvert.h.
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief converting a bsdf_ptr to and from a string

    Define string_convert outside bsdf_ptr.h due to the circular relation
    between aggregatebsdf and bsdf_ptr.
    *********************************************************************/
  template<typename BSDF_PTR> requires concepts::bsdf_ptr<BSDF_PTR>
    struct string_converter<BSDF_PTR>
  {
    BBM_IMPORT_CONFIG( BSDF_PTR );

    /********************************************************************/
    /*! \brief toString: forward to toStrig method of bsdf_ptr
     ********************************************************************/
    static inline std::string toString(const BSDF_PTR& obj) { return obj.toString(); }

    /********************************************************************/
    /*! \brief fromString converts a string to a bsdf_ptr to a bsfmodel.

      Strategy:
      
      1. created a sorted named tuple where the name is the bsdfmodel, and the
      type is the function call to the corresponding fromString<bsdfmodel>

      2. perform a binary search to match the key (object name in string) to
      the name of the bsdfmodels.

      3. Once found, create the bsdfmodel and correspodning bsdf_ptr.

      To do: Speed up compilation: this code for some reason compiles very
      slowly (multiple seconds) when called for the first time in the
      translation unit.
    *********************************************************************/
    static inline BSDF_PTR fromString(const std::string& str)
    {
      #include "export/bbm_fromstring.h"
      
      // create lookup at compile time as a sorted named tuple
      auto dict = sort_named( named_cat(
                                        #include "bbm_bsdfmodels.h"
                                        make_named<aggregatebsdf<Config>::name>( bbm::fromString<aggregatebsdf<Config>> ) // Needs aggregatebsdf instead of aggregatemodel
                                        ) );
      
      #include "export/clear_export.h"

      // get keyword
      auto [key, value] = string::get_keyword(str);

      // binary search
      return binary_search_named(key, dict, []<size_t IDX, typename NAMED>(const std::string& key, NAMED&& named, const std::string& str) -> BSDF_PTR
      {
        // if not found (outside range) => throw exception
        if constexpr (IDX == std::decay_t<NAMED>::size) throw std::invalid_argument(std::string("BBM: unrecognized bsdf_ptr type ") + key + " in: " + str);
        else
        {
          // if not found (key does not match) => throw throw exception
          if(key != std::string(std::decay_t<NAMED>::template name<IDX>)) throw std::invalid_argument(std::string("BBM: unrecognized bsdf_ptr type ") + key + " in: " + str);

          // if found; call the corresponding (stored) fromString to create the BSDF_PTR.
          else return make_bsdf_ptr(std::get<IDX>(std::forward<NAMED>(named))(str));
        }
      }, str);
      
      // Done.
    }
  };

} // end bbm namespace

#endif /* _BBM_BSDF_STRING_CONVERT_H_ */
