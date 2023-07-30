#ifndef _BBM_MASKINGSHADOWING_VANGINNEKEN_H_
#define _BBM_MASKINGSHADOWING_VANGINNEKEN_H_

#include "bbm/maskingshadowing.h"

/************************************************************************/
/*! \file ginneken.h

  \brief Height-direction correlated joint masking and shadowing following the
  empirical factor from "Diffuse and Specular Reflectance from Rough
  Surfaces," by Bram van Ginneken, Marigo Stavridi, and Jan J. Koenderink,
  Appl. Opt. 37, 130-139 (1998):
  https://opg.optica.org/ao/abstract.cfm?URI=ao-37-1-130

  See also Eq. 101 from "Understanding the Masking-Shadowing Function in
  Microfacet-Based BRDFs" [Heitz 2014]: https://jcgt.org/published/0003/02/03/

*************************************************************************/

namespace bbm {
  namespace maskingshadowing {

    /********************************************************************/
    /*! \brief Heigh correlated joint masking and shadowing following
      Vanginneken et al.

      See also Eq. 101 from "Understanding the Masking-Shadowing Function in
      Microfacet-Based BRDFs" [Heitz 2014]:
      https://jcgt.org/published/0003/02/03/
    ********************************************************************/
    template<typename CONF> requires concepts::config<CONF>
      struct vanginneken
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

        // Van Ginnekin empirical factor:
        //   lambda = 4.41 phi / (4.441 phi + 1)
        // with phi = the azimuthal angle between in and out.
        auto phi = bbm::abs(spherical::phi(in) - spherical::phi(out));
        return_t lambda = 4.41 * phi / (4.41 * phi + 1.0);
        
        // Compute Eq. 101
        auto gi = ndf.G1(in, m, mask);
        auto go = ndf.G1(out, m, mask);
        auto gio = gi*go;
        
        // min/max(delta_i, delta_o) = min/max(1/gi - 1, 1/go - 1)
        //                           = 1 / (max/min(gi,go)) - 1
        //                           = min/max(gi,go) / gio - 1
        auto maxg = bbm::max(gi, go);
        auto ming = bbm::min(gi, go);

        // denom = 1 + max(delta_i, delta_o) + lambda * min(delta_i, delta_o)
        //       = 1 + (max(gi, go)/gio - 1) + lambda*( min(gi,go)/gio - 1)
        //        = (max(gi,go)+ lambda*(min(gi,go) - gio)) / gio
        auto denom = maxg + lambda*(ming - gio);

        // check for division by zero
        mask &= (denom > Constants::Epsilon());
        
        // Done.
        return bbm::select(mask, gio / denom, 0);
      }
    };

    BBM_CHECK_CONCEPT( concepts::maskingshadowing, vanginneken<config> );
    
  } // end maskingshadowing namespace
} // end bbm namespace

#endif /* _BBM_MASKINGSHADOWING_VANGINNEKEN_H_ */
