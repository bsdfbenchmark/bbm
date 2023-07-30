#ifndef _BBM_MASKINGSHADOWING_UNCORRELATED_H_
#define _BBM_MASKINGSHADOWING_UNCORRELATED_H_

#include "bbm/maskingshadowing.h"

/************************************************************************/
/*! \file uncorrelated.h

  \brief Uncorrelated joint masking and shadowing. Follows Eq. 98 from
  "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
  [Heitz 2014]: https://jcgt.org/published/0003/02/03/
*************************************************************************/
  
namespace bbm {
  namespace maskingshadowing {

    /*******************************************************************/
    /*! \brief Uncorrelated joint masking and shadowing.

      Follows Eq. 98 from "Understanding the Masking-Shadowing Function in
      Microfacet-Based BRDFs" [Heitz 2014]:
      https://jcgt.org/published/0003/02/03/
    ********************************************************************/
    template<typename CONF> requires concepts::config<CONF>
      struct uncorrelated
    {
      BBM_IMPORT_CONFIG( CONF );

      template<typename NDF> requires concepts::ndf<NDF> && concepts::matching_config<CONF, NDF>
        static constexpr auto eval(const NDF& ndf, const Vec3d& in, const Vec3d& out, const Vec3d& m, Mask mask=true)
      {
        using return_t = decltype(ndf.G1(in, m, mask));
        
        // check dot
        mask &= (bbm::dot(in, m) > 0) && (bbm::dot(out, m) > 0);

        // Quick bailout
        if(bbm::none(mask)) return return_t(0);

        // Done.
        return bbm::select(mask, ndf.G1(in, m, mask) * ndf.G1(out, m, mask), 0);
      }
    };

    BBM_CHECK_CONCEPT( concepts::maskingshadowing, uncorrelated<config> );
    
  } // end maskingshadowing namespace
} // end bbm namespace

#endif /* _BBM_MASKINGSHADOWING_UNCORRELATED_H_ */
