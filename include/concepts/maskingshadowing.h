#ifndef _BBM_MASKINGSHADOWING_CONCEPT_H_
#define _BBM_MASKINGSHADOWING_CONCEPT_H_

#include "concepts/ndf.h"
#include "concepts/util.h"

/************************************************************************/
/*! \file maskingshadowing.h

  \brief Joint Masking-Shadowing Functor concept
*************************************************************************/

namespace bbm {
  namespace concepts {

    /********************************************************************/
    /*! \brief maskingshadowing concept

      Each Masking-Shadowing requires:
      + concepts::has_config
      + static Value/Spectrum eval(const NDF&, const Vec3d&, const Vec3d& in, const Vec3d& out, const Vec3d& m, Mask mask=true)
    *********************************************************************/
    template<typename MS>
      concept maskingshadowing = requires
    {
      requires concepts::has_config<MS>;

      /******************************************************************/
      /*! \brief Evaluate the masking-shaodinw function for a given ndf, and in/out/NDF normal vector.

        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
        template<typename NDF> requires concepts::matching_config<NDF, MS>
          Value/Spectrum eval(const NDF& ndf, const Vec3d& in, const Vec3d& out, const Vec3d& m, Mask mask=true) const
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        \param ndf = NDF to compute masking-shadowing for
        \param in = incident direction of transport
        \param out = exitant direction of transport
        \param m = microfacet normal
        \param mask = enable/disable lanes
        \returns shadowing and masking ratio.
      *******************************************************************/
      { std::decay_t<MS>::eval(std::declval<archetype::ndf<get_config<MS>>>(), std::declval<Vec3d_t<MS>>(), std::declval<Vec3d_t<MS>>(), std::declval<Vec3d_t<MS>>(), std::declval<Mask_t<MS>>()) } -> concepts::same_as_any<Value_t<MS>, Spectrum_t<MS>>;

      //! \brief mask has default value
      { std::decay_t<MS>::eval(std::declval<archetype::ndf<get_config<MS>>>(), std::declval<Vec3d_t<MS>>(), std::declval<Vec3d_t<MS>>(), std::declval<Vec3d_t<MS>>()) } -> concepts::same_as_any<Value_t<MS>, Spectrum_t<MS>>;
    };

    /********************************************************************/
    /*! \brief maskingshadowing archetype for concept checking

      \tparam CONF = config to check for. Default: archetype::config
      \tparam RET = return type (spectrum or value)
    *********************************************************************/
    namespace archetype {
      template<typename CONF=config, typename RET=Value_t<CONF>> requires concepts::config<CONF>
        struct maskingshadowing
      {
        using Config = CONF;
        static RET eval(ndf<Config>, Vec3d_t<Config>, Vec3d_t<Config>, Vec3d_t<Config>, Mask_t<Config> = true);
      };
    } // end archetype namespace

    BBM_CHECK_CONCEPT(concepts::maskingshadowing, archetype::maskingshadowing<>);
    
  } // end concepts namespace
} // end bbm namespace

#endif /* _BBM_MASKINGSHADOWING_CONCEPT_H_ */
