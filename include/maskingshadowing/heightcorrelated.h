#ifndef _BBM_MASKINGSHADOWING_HEIGHT_CORRELATED_H_
#define _BBM_MASKINGSHADOWING_HEIGHT_CORRELATED_H_

#include "bbm/maskingshadowing.h"

/************************************************************************/
/*! \file heightcorrelated.h

  \brief Height correlated joint masking and shadowing. Follows Eq. 99 from
  "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
  [Heitz 2014]: https://jcgt.org/published/0003/02/03/
*************************************************************************/
  
namespace bbm {
  namespace maskingshadowing {

    /********************************************************************/
    /*! \brief Height correlated joint masking and shadowing.

      Follows Eq. 99 from "Understanding the Masking-Shadowing Function in
      Microfacet-Based BRDFs" [Heitz 2014]:
      https://jcgt.org/published/0003/02/03/
    *********************************************************************/
    template<typename CONF> requires concepts::config<CONF>
      struct heightcorrelated
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

        // Compute Eq 99 = (1 + delta_i + delta_o)^-1
        //               = (1 + (1/g_i - 1) + (1/g_o - 1))^-1
        //               = (1/g_i + 1/g_o - 1)^-1
        //               = g_i*g_o / (g_i + g_o - g_i*g_o)
        auto gi = ndf.G1(in, m, mask);
        auto go = ndf.G1(out, m, mask);
        auto gio = gi*go;
        auto denom = gi + go - gio;

        // check for division by zero
        mask &= (denom > Constants::Epsilon());
        
        // Done.
        return bbm::select(mask, gio / denom, 0);
      }
    };

    BBM_CHECK_CONCEPT( concepts::maskingshadowing, heightcorrelated<config> );
    
  } // end maskingshadowing namespace
} // end bbm namespace

#endif /* _BBM_MASKINGSHADOWING_UNCORRELATED_H_ */
