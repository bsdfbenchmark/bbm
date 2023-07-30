#ifndef _BBM_MASKINGSHADOWING_VGROOVE_H_
#define _BBM_MASKINGSHADOWING_VGROOVE_H_

#include "bbm/maskingshadowing.h"

/************************************************************************/
/*! \file vgroove.h

  \brief Vgroove shadowing and masking. Based on Torrance and Sparrow, 19967,
  "Theory for off-specular reflection from roughened surfaces":
  https://dl.acm.org/doi/10.5555/136913.136924
*************************************************************************/
  
namespace bbm {
  namespace maskingshadowing {

    /********************************************************************/
    /*!   \brief Vgroove shadowing and masking.

      Based on Torrance and Sparrow, 19967, "Theory for off-specular
      reflection from roughened surfaces":
      https://dl.acm.org/doi/10.5555/136913.136924
    *********************************************************************/
    template<typename CONF> requires concepts::config<CONF>
      struct vgroove
    {
      BBM_IMPORT_CONFIG( CONF );

      template<typename NDF> requires concepts::ndf<NDF> && concepts::matching_config<CONF, NDF>
        static constexpr Value eval(const NDF& ndf, const Vec3d& in, const Vec3d& out, const Vec3d& m, Mask mask=true)
      {
        using return_t = decltype(ndf.G1(in, m, mask));
        
        // check dot
        mask &= (bbm::dot(in, m) > 0) && (bbm::dot(out, m) > 0);

        // Quick bailout
        if(bbm::none(mask)) return return_t(0);

        // eval.
        auto G = bbm::min( 1.0, bbm::min(
                           2.0 * vec::z(m) * vec::z(in) / bbm::dot(in, m),
                           2.0 * vec::z(m) * vec::z(out) / bbm::dot(out, m)) );

        // Done.
        return return_t( bbm::select(mask, G, 0) );
      }
    };

    BBM_CHECK_CONCEPT( concepts::maskingshadowing, vgroove<config> );
    
  } // end maskingshadowing namespace
} // end bbm namespace

#endif /* _BBM_MASKINGSHADOWING_VGROOVE_H_ */
