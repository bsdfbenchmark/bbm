#ifndef _BBM_CONSTRUCTOR_CONCEPT_H_
#define _BBM_CONSTRUCTOR_CONCEPT_H_

#include "core/args.h"

/************************************************************************/
/*! \file constructor.h

  \brief bbm constructor concept
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief constructor concept

      A bbm constructor requires:
      + typename constructor_args_t that is an bbm::args
      + the object is constructivle from an constructor_args_t
    *********************************************************************/
    template<typename OBJ>
      concept constructor = requires
    {
      requires bbm::is_args_v< typename std::decay_t<OBJ>::constructor_args_t >;
      requires std::is_constructible_v<OBJ, typename std::decay_t<OBJ>::constructor_args_t>;
    };

  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_CONSTRUCTOR_CONCEPT_H_ */
