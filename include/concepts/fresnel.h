#ifndef _BBM_FRESNEL_CONCEPT_H_
#define _BBM_FRESNEL_CONCEPT_H_

#include "bbm/config.h"
#include "core/ior.h"

/************************************************************************/
/*! \file fresnel.h

  \brief fresnel contract

*************************************************************************/

namespace bbm {
  namespace concepts {

    /*******************************************************************/
    /*! \brief fresnel concept

      Each fresnel implementation has:
      + concepts::config
      + typename parameter_type
      + static Value|Spectrum eval(const parameter_type& eta, const Value& cosTheta, Mask Mask=true)
      
    *********************************************************************/
    template<typename Fresnel>
      concept fresnel = requires
    {
      requires concepts::config<Fresnel>;
      typename std::decay_t<Fresnel>::parameter_type;
      
      //! \brief eval method
      { std::decay_t<Fresnel>::eval( std::declval<typename std::decay_t<Fresnel>::parameter_type>(), std::declval<Value_t<Fresnel>>(), std::declval<Mask_t<Fresnel>>() ) } -> concepts::same_as_any<Value_t<Fresnel>, Spectrum_t<Fresnel>>;

      //! \brief eval method with defaulted mask
      { std::decay_t<Fresnel>::eval( std::declval<typename std::decay_t<Fresnel>::parameter_type>(), std::declval<Value_t<Fresnel>>() ) } -> concepts::same_as_any<Value_t<Fresnel>, Spectrum_t<Fresnel>>;
     };

    /********************************************************************/
    /*! \brief fresnel archetype for concept checking

      \tparam CONF = config to check for. Default value = archetype::config
      \tparam PARAM = parameter type (ior)
    *********************************************************************/
    namespace archetype {
      template<typename CONF=config, typename PARAM=ior::ior<Value_t<CONF>>> requires concepts::config<CONF> && ior::is_ior_type<PARAM>
        struct fresnel
      {
        using Config = CONF;
        using parameter_type = PARAM;
        static Value_t<Config> eval(parameter_type, Value_t<Config>, Mask_t<Config> = true);
      };
    } // end archetype namespace

    BBM_CHECK_CONCEPT(concepts::fresnel, archetype::fresnel<>);
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_FRESNEL_CONCEPT_H_ */
