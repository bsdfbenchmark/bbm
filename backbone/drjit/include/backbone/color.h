#ifndef _BBM_DRJIT_COLOR_H_
#define _BBM_DRJIT_COLOR_H_

/************************************************************************/
/*! \file color.h

  \brief Defines a color as an alias to a drjit array.
*************************************************************************/

namespace backbone {

  template<typename T>
    using color = drjit::Array<T, 3>;

} // end backbone namespace

#endif /* _BBM_DRJIT_COLOR_H_ */

