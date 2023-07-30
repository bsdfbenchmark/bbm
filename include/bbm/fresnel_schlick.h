#ifndef _BBM_FRESNEL_SCHLICK_H_
#define _BBM_FRESNEL_SCHLICK_H_

#include "concepts/fresnel.h"
#include "bbm/config.h"

namespace bbm {
  namespace fresnel {

    /*******************************************************************/
    /*! \brief Implements the Fresnel reflectance equation as proposed by
        Schlick [Comp. Graph. Forum '94]:
        https://doi.org/10.1111%2F1467-8659.1330233

      \tparam CONF = bbm configuration

      Implements: concepts::fresnel
    ********************************************************************/
    template<typename CONF, typename PARAM=ior::reflectance<Value_t<CONF>>> requires concepts::config<CONF>
      struct schlick
    {
      BBM_IMPORT_CONFIG( CONF );

      //! \brief Fresnel Parameter Type
      using parameter_type = PARAM;
      
      /*****************************************************************/
      /*! \brief Evaluate Fresnel reflectance 

        This method follows Schlick's approximation [Comp. Graph. Forum' 94]
        of the Fresnel reflectance
        (https://doi.org/10.1111%2F1467-8659.1330233). The method takes either
        ior or reflectance (relies on auto-conversion to reflectance), as well
        as the dot product between view and halfway vector (cosTheta) as
        input.

        \param param = reflectance at normal incidence
        \param cosTheta = dot product between view and halfway.
        \param mask = mask compute lines
        \returns Approximate Fresnel reflectance
      *******************************************************************/
      static constexpr typename std::decay_t<parameter_type>::type eval(const parameter_type& param, const Value& cosTheta, Mask mask=true)
      {
        // quick bail out
        if(bbm::none(mask)) return 0;

        // do computations with ior::reflectance
        ior::reflectance<typename parameter_type::type> R0(param);
        
        // Done.
        return bbm::select(mask, R0 + (Scalar(1.0) - R0) * bbm::pow(Scalar(1.0) - cosTheta, 5.0), 0);
      }
    };

    // check Value_t variant
    BBM_CHECK_CONCEPT( concepts::fresnel, schlick<config, ior::ior<Value_t<config>>> );
    BBM_CHECK_CONCEPT( concepts::fresnel, schlick<config, ior::reflectance<Value_t<config>>> );
    
    // check Spectrum_t variant
    BBM_CHECK_CONCEPT( concepts::fresnel, schlick<config, ior::ior<Spectrum_t<config>>> );
    BBM_CHECK_CONCEPT( concepts::fresnel, schlick<config, ior::reflectance<Spectrum_t<config>>> );
    
  } // end fresnel namespace
} // end bbm namespace

        
#endif /* _BBM_FRESNEL_SCHLICK_H_ */
