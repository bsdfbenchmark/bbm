#ifndef _BBM_FRESNEL_COOK_H_
#define _BBM_FRESNEL_COOK_H_

#include "concepts/fresnel.h"
#include "bbm/config.h"

namespace bbm {
  namespace fresnel {

    /*******************************************************************/
    /*! \brief Implements the Fresnel reflectance equation as proposed by Cook
        and Torrance [SIGGRAPH'82]: https://doi.org/10.1145/357290.357293

        \tparam CONF = bbm configuration
        \tparam PARAM = parameter type (ior)
        
      Implements: concepts::fresnel
    ********************************************************************/
    template<typename CONF, typename PARAM=ior::ior<Value_t<CONF>>> requires concepts::config<CONF> 
      struct cook
    {
      BBM_IMPORT_CONFIG( CONF );

      //! \brief Fresnel Parameter Type
      using parameter_type = PARAM;
      
      /*****************************************************************/
      /*! \brief Evaluate Fresnel reflectance 

        This method follows Cook and Torrance's [SIGGRAPH' 82] suggested
        evaluation of the Fresnel reflectance
        (https://doi.org/10.1145/357290.357293). The method takes either ior
        or reflectance (relies on auto-conversion to ior), as well as the dot
        product between view and halfway vector (cosTheta) as input.

        \param param = index of refraction 
        \param cosTheta = dot product between view and halfway.
        \param mask = mask compute lines
        \returns Fresnel reflectance with the type determined from the underlying type of the parameter.
      *******************************************************************/
      static constexpr typename std::decay_t<parameter_type>::type eval(const parameter_type& param, const Value& cosTheta, Mask mask=true) 
      {
         // quick bail out
        if(bbm::none(mask)) return 0;

        // do computations with ior::ior
        ior::ior<typename parameter_type::type> eta(param);
        
        // compute
        auto g = bbm::safe_sqrt( eta*eta + cosTheta*cosTheta - Scalar(1.0) );
        auto a = (g - cosTheta) / (g + cosTheta);
        auto b = (cosTheta*(g+cosTheta) - Scalar(1.0)) / (cosTheta*(g-cosTheta) + Scalar(1.0));

        // Done.
        return bbm::select(mask, bbm::max(Scalar(0.5) * (a*a) * (Scalar(1.0) + b*b), 0.0), 0.0);
      }

    };

    // check Value_t variants
    BBM_CHECK_CONCEPT( concepts::fresnel, cook<config, ior::ior<Value_t<config>>> );
    BBM_CHECK_CONCEPT( concepts::fresnel, cook<config, ior::reflectance<Value_t<config>>> );

    // check Spectrum_t variants
    BBM_CHECK_CONCEPT( concepts::fresnel, cook<config, ior::ior<Spectrum_t<config>>> );
    BBM_CHECK_CONCEPT( concepts::fresnel, cook<config, ior::reflectance<Spectrum_t<config>>> );
    
  } // end fresnel namespace
} // end bbm namespace


#endif /* _BBM_FRESNEL_COOK_H_ */
