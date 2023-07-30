#ifndef _BBM_UNIT_H_
#define _BBM_UNIT_H_

#include <array>
#include <type_traits>
#include "util/flags.h"

/************************************************************************/
/*! \file unit.h
    \brief Light unit: Radiance or Importance

    unit_t::Radiance
    unit_t::Importance

    unit_t::Adjoint is a alias for unit_t::Importance
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Light Unit
   **********************************************************************/
  enum struct unit_t
  {
    Radiance   = 0x0000,
    Importance = 0x0001,
    Adjoint = Importance,
  };

  static constexpr std::array all_unit_t{unit_t::Radiance, unit_t::Importance};
  

  /////////////////////
  // ostream support //
  /////////////////////
  std::ostream& operator<<(std::ostream& s, const bbm::unit_t& unit)
  {
    switch(unit)
    {
      case bbm::unit_t::Radiance :   { s << "Radiance";      break; }
      case bbm::unit_t::Importance : { s << "Importance";    break; }
    }
    return s;
  }

} // end bbm namespace
  
#endif /* _BBM_UNIT_H_ */
