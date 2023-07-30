#ifndef _BBM_BSDFSAMPLE_H_
#define _BBM_BSDFSAMPLE_H_

#include <ostream>

#include "util/reflection.h"
#include "bbm/config.h"

/******************************************************************************/
/*! \file bsdfsample.h
    \brief Structure to hold a sampled direction and corresponding pdf.
*******************************************************************************/

namespace bbm {

  /*********************************************************************/
  /*! \brief Structure to hold a sample's direction and PDF
   *********************************************************************/    
  template<typename CONF> requires concepts::config<CONF>
  struct bsdfsample
  {
    BBM_IMPORT_CONFIG( CONF );

    Vec3d direction;     ///< Sampled direction
    Value pdf;           ///< Pdf of the sampled direction
    BsdfFlag flag;       ///< Type of sample

    BBM_ATTRIBUTES(direction, pdf, flag);
  };

  /////////////////////
  // ostream support //
  /////////////////////
  template<typename CONF>
    std::ostream& operator<<(std::ostream& s, const bbm::bsdfsample<CONF>& ss)
  {
    return s << "(" << ss.direction << ", " << ss.pdf << ", " << ss.flag << ")";
  }
  
} // end bbm namespace
  
#endif /* _BBM_BSDFSAMPLE_H_ */
