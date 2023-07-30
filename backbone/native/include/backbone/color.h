#ifndef _BBM_NATIVE_COLOR_H_
#define _BBM_NATIVE_COLOR_H_

#include "backbone/array.h"

/************************************************************************/
/*! \file color.h

  \brief Define an RGB color as a specialization of array

*************************************************************************/

namespace backbone {

  template<typename T>
    using color = array<T, 3>;

} // end backbone namespace

#endif /* _BBM_NATIVE_COLOR_H_ */
