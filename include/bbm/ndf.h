#ifndef _BBM_NDF_H_
#define _BBM_NDF_H_

#include "concepts/macro.h"
#include "concepts/ndf.h"
#include "concepts/reflection.h"

#include "util/toString.h"

#include "core/spherical.h"
#include "core/transform.h"
#include "core/vec_transform.h"
#include "core/shading_frame.h"

#include "bbm/config.h"
#include "bbm/constructor.h"
#include "bbm/bsdf_attribute.h"

/************************************************************************/
/*! \file ndf.h

  \brief All includes and helpers needed for declaring new ndfs

*************************************************************************/

namespace bbm {

  //////////////////////
  // ostream operator
  //////////////////////
  template<typename NDF> requires bbm::concepts::ndf<NDF>
    std::ostream& operator<<(std::ostream& s, const NDF& ndf)
  {
    s << NDF::name;

    // toString method has preference
    if constexpr (has_toString<NDF>) s << ndf.toString();

    // other use reflection if available
    else if constexpr (bbm::concepts::reflection::supported<NDF>) s << bbm::reflection::attributes(ndf);

    // else empty '()'
    else s << "()";

    // Done.
    return s;
  }

} // end bbm namespace

#endif /* _BBM_NDF_H_ */
