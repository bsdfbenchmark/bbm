#ifndef _BBM_ENOKI_COLOR_H_
#define _BBM_ENOKI_COLOR_H_

#include "backbone/array.h"

/***********************************************************************/
/*! \file color.h
  \brief Defines a color as an alias to an enoki array.
************************************************************************/

namespace backbone {

    template<typename T>
      using color = enoki::Array<T, 3>;

} // end backbone namespace

#endif /* _BBM_ENOKI_COLOR_H_ */
