#ifndef _BBM_FRESNEL_COMPLEX_H_
#define _BBM_FRESNEL_COMPLEX_H_

#include "concepts/fresnel.h"
#include "bbm/config.h"

namespace bbm {
  namespace fresnel {

    template<typename CONF, typename VALUE=Value_t<CONF>> requires concepts::config<CONF>
      struct complex
    {
      BBM_IMPORT_CONFIG( CONF );

      //! \brief Fresnel Parameter Type
      using parameter_type = ior::complex_ior<VALUE>;

      /******************************************************************/
      /*! \brief

        "Physically based Lighting Calculations for Computer Graphics"
        [Shirley 1985]: https://dl.acm.org/doi/10.5555/124947

        p15, Eqs. 2.4-2.7 for dielectric-conductor interfaces (assuming
        dielitric is air with ior=1).
        
        Implements: concepts::fresnel
       ******************************************************************/
      static constexpr VALUE eval(const parameter_type& param, const Value& cosTheta, Mask mask=true)
      {
        auto cosTheta2 = cosTheta*cosTheta;
        auto sinTheta2 = 1 - cosTheta2;
        
        auto n = real(param), n2 = n*n;
        auto k = imag(param), k2 = k*k;
        
        // Compute a, and a2 + b2 (Eqs. 2.6 & 2.7)
        auto temp = n2 - k2 - sinTheta2;
        auto a2b2 = bbm::safe_sqrt( temp*temp + 4*n2*k2 );
        auto a = bbm::safe_sqrt(0.5 * (a2b2 + temp));

        // Compute Rs (Eq. 2.4)
        auto a2c = 2*a*cosTheta;
        auto Rs = (a2b2 - a2c + cosTheta2) / (a2b2 + a2c + cosTheta2);

        // Compute Rp (Eq. 2.5; replace tanTheta by sinTheta/cosTheta)
        auto Rp = Rs * (cosTheta2*a2b2 - (a2c - sinTheta2)*sinTheta2) / (cosTheta2*a2b2 + (a2c + sinTheta2)*sinTheta2);

        // Done.
        return bbm::select(mask, 0.5*(Rs+Rp), 0.0);
      }
    };

    BBM_CHECK_CONCEPT( concepts::fresnel, complex<config, Value_t<config>> );
    BBM_CHECK_CONCEPT( concepts::fresnel, complex<config, Spectrum_t<config>> );
    
  } // end fresnel namespace
} // end bbm namespace

#endif /* _BBM_FRESNEL_COMPLEX_H_ */
