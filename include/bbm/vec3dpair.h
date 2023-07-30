#ifndef _BBM_VEC3DPAIR_H_
#define _BBM_VEC3DPAIR_H_

#include <ostream>

#include "util/reflection.h"
#include "bbm/config.h"

/************************************************************************/
/*! \file vec3dpair.h
    \brief Structure to hold a pair of directions
*************************************************************************/

namespace bbm {

  /**********************************************************************/
  /*! \brief Structure to hold a pair of directions
  ***********************************************************************/
  template<typename CONF> requires concepts::config<CONF>
    struct vec3dpair
  {
    BBM_IMPORT_CONFIG( CONF );

    Vec3d in;            ///< In direction
    Vec3d out;           ///< Out direction

    BBM_ATTRIBUTES(in, out);
  };


  /////////////////////
  // ostream support //
  /////////////////////
  template<typename CONF>
  std::ostream& operator<<(std::ostream& s, const bbm::vec3dpair<CONF>& rs)
  {
    return s << "(" << rs.in << ", " << rs.out << ")";
  }

} // end bbm namespace
  
#endif /* _BBM_VEC3DPAIR_H_ */
