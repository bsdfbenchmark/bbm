#ifndef _BBM_ENOKI_HORIZONTAL_H_
#define _BBM_ENOKI_HORIZONTAL_H_

#include "backbone/array.h"

/************************************************************************/
/*! \file horizontal.h

  \brief Map bbm horizontal methods to corresponding enoki methods

*************************************************************************/

namespace backbone {

    /********************************************************************/
    /*! @{ Direct mapping to Enoki methods
      *******************************************************************/
    using enoki::hsum;
    using enoki::hprod;
    using enoki::hmax;
    using enoki::hmin;
    using enoki::dot;
    using enoki::norm;
    using enoki::squared_norm;
    using enoki::normalize;
    using enoki::all;
    using enoki::any;
    using enoki::none;
    using enoki::count;
    //! @}
    
} // end backbone namespace

#endif /* _BBM_ENOKI_HORIZONTAL_H_ */
