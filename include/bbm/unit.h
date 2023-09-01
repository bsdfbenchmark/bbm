#ifndef _BBM_UNIT_H_
#define _BBM_UNIT_H_

#include <type_traits>
#include "util/flags.h"

/************************************************************************/
/*! \file unit.h
    \brief Light unit: Radiance or Importance

    unit_t::Radiance
    unit_t::Importance
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Light Unit
   **********************************************************************/
  enum struct unit_t
  {
    Radiance   = 0x0000,
    Importance = 0x0001,
  };

  BBM_ENUM(unit_t, Radiance, Importance);
  
} // end bbm namespace
  
#endif /* _BBM_UNIT_H_ */
