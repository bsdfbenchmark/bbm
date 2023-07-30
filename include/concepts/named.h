#ifndef _BBM_NAMED_CONCEPT_H_
#define _BBM_NAMED_CONCEPT_H_

#include "concepts/util.h"
#include "util/string_literal.h"

/************************************************************************/
/*!  \file named.h
  \brief named (class) concept. 
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief A named class is a class that contains a static constexpr string_literal name.
     ********************************************************************/
    template<typename T>
      concept named = requires
    {
      { std::decay_t<T>::name } -> trait_wrapper<is_string_literal>;
    };

  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_NAMED_CONCEPT_H_ */

  
